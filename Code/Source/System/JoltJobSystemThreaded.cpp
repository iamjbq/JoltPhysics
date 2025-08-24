
#include "JoltJobSystemThreaded.h"

#include <AzCore/Debug/Profiler.h>
#include <AzCore/Jobs/JobManager.h>
#include <AzCore/Jobs/JobContext.h>

namespace JoltPhysics
{
    JoltJobSystemThreaded::JoltJobSystemThreaded(JPH::uint inMaxJobs, JPH::uint inMaxBarriers, int inNumThreads)
    {
        Init(inMaxJobs, inMaxBarriers, inNumThreads);
    }

    JoltJobSystemThreaded::~JoltJobSystemThreaded()
    {
        m_jobContext.reset();
        m_jobManager.reset();
    }

    // With the O3DE job system, we shouldn't need to manage a queue manually like the example JobSystemThreadPool
    void JoltJobSystemThreaded::Init(JPH::uint inMaxJobs, JPH::uint inMaxBarriers, int inNumThreads)
    {
        JobSystemWithBarrier::Init(inMaxBarriers);

        // Init freelist of jobs to track job lifetime
        mJobs.Init(inMaxJobs, inMaxJobs);
        
        // Auto-detect number of threads
        if (inNumThreads < 0)
            m_numWorkerThreads = AZStd::thread::hardware_concurrency();
        
        // If no threads are requested we're done
        if (inNumThreads == 0)
            return;

        // Otherwise, set manually
        if (inNumThreads > 0)
            m_numWorkerThreads = static_cast<unsigned int>(inNumThreads);

        AZ::JobManagerDesc jobDesc;
        for (size_t i = 0; i < m_numWorkerThreads; ++i)
        {
            jobDesc.m_workerThreads.push_back({ static_cast<int>(i) });
        }
        // m_jobManager = AZStd::make_unique<AZ::JobManager>(jobDesc);
        m_jobManager = AZStd::shared_ptr<AZ::JobManager>(aznew AZ::JobManager(jobDesc));
        // m_jobContext = AZStd::make_unique<AZ::JobContext>(*m_jobManager);
        m_jobContext = AZStd::shared_ptr<AZ::JobContext>(aznew AZ::JobContext(*m_jobManager));
    }

    JPH::JobHandle JoltJobSystemThreaded::CreateJob(const char* inName, JPH::ColorArg inColor, const JobFunction& inJobFunction, JPH::uint32 inNumDependencies)
    {
        // Loop until we can get a job from the free list
        JPH::uint32 index;
        for (;;)
        {
            index = mJobs.ConstructObject(inName, inColor, this, inJobFunction, inNumDependencies);
            if (index != AvailableJobs::cInvalidObjectIndex)
                break;
            JPH_ASSERT(false, "No jobs available!");
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        Job* job = &mJobs.Get(index);

        // Construct handle to keep a reference, the job is queued below and may immediately complete
        JobHandle handle(job);

        // If there are no dependencies, queue the job now
        if (inNumDependencies == 0)
            QueueJob(job);

        // Return the handle
        return handle;
    }

    void JoltJobSystemThreaded::QueueJob(Job* inJob)
    {
        QueueJobInternal(inJob);
    }

    void JoltJobSystemThreaded::QueueJobs(Job** inJobs, JPH::uint inNumJobs)
    {
        // Queue all jobs
        for (Job** job = inJobs, **job_end = inJobs + inNumJobs; job < job_end; ++job)
            QueueJobInternal(*job);
    }

    void JoltJobSystemThreaded::QueueJobInternal(Job* inJob)
    {
        // Add reference to job because we're adding the job to the queue
        inJob->AddRef();

        // Not ideal, but since JPH::Job is protected, we have to make another handle to pass into our JoltJob wrapper
        // We could maybe just pass the JPH::JobFunction into an anonymous AZ::Job lamba, but we'll see
        JobHandle handle(inJob);

        // Might not want to auto-delete job if it interferes with tracking barrier jobs
        JoltJob* jobWrapper = aznew JoltJob(handle, m_jobContext.get());
        jobWrapper->Start();
    }
    
    void JoltJobSystemThreaded::FreeJob(Job* inJob)
    {
        mJobs.DestructObject(inJob);
    }

    JoltJobSystemThreaded::JoltJob::JoltJob(JPH::JobHandle& inJobHandle, AZ::JobContext* context)
        : AZ::Job(true, context)
        , m_jobHandle(inJobHandle)
    {
    }

    void JoltJobSystemThreaded::JoltJob::Process()
    {
        AZ_PROFILE_SCOPE(Physics, "Jolt Queued Job");

        auto* jobPtr = m_jobHandle.GetPtr();
        
        // If the job was sent to a Barrier even without dependencies, we need to make sure it wasn't already executed
        if (jobPtr != nullptr && jobPtr->CanBeExecuted())
        {
            jobPtr->Execute();
            jobPtr->Release();
        }
        
    }

}

