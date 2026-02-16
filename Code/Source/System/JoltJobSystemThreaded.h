
#pragma once

#include <AzCore/Jobs/Job.h>
#include <AzCore/Jobs/JobContext.h>
#include <AzCore/Jobs/JobManager.h>

#include <Jolt/Jolt.h>
#include "Jolt/Core/FixedSizeFreeList.h"
#include "Jolt/Core/JobSystemWithBarrier.h"

namespace JoltPhysics
{
    //! Named system allocator specifically for Jolt job system work.
    // AZ_CHILD_ALLOCATOR_WITH_NAME(JoltAllocator, "JoltAllocator", "{3B69EBB9-A4A3-4E3A-97D2-17B3536D6578}", AZ::SystemAllocator);

    class JoltJobSystemThreaded
        : public JPH::JobSystemWithBarrier
    {
    public:
        AZ_CLASS_ALLOCATOR(JoltJobSystemThreaded, AZ::SystemAllocator);

        JoltJobSystemThreaded(JPH::uint inMaxJobs, JPH::uint inMaxBarriers, int inNumThreads = -1);
        JoltJobSystemThreaded() = default;
        ~JoltJobSystemThreaded() override;

        void Init(JPH::uint inMaxJobs, JPH::uint inMaxBarriers, int inNumThreads = -1);
        
        int GetMaxConcurrency() const override { return static_cast<int>(m_numWorkerThreads); }
        
        // Creates a Job and returns a prepared JobHandle for adding to a Barrier for execution.
        // If the job has no dependencies, it will be immediately queued for execution using O3DE's job system.
        // Created Jobs are tracked between Jolt's internal Barrier system and O3DE's job system to prevent double execution.
        JobHandle CreateJob(const char* inName, JPH::ColorArg inColor, const JobFunction& inJobFunction, JPH::uint32 inNumDependencies) override;

        // Queues a Job for immediate execution using O3DE's job system.
        void QueueJob(Job* inJob) override;

        // Queues multiple Jobs for immediate execution using O3DE's job system.
        void QueueJobs(Job** inJobs, JPH::uint inNumJobs) override;

        // Removes Job from the job pointer array. 
        void FreeJob(Job* inJob) override;

    protected:
        class JoltJob : public AZ::Job
        {
        public:
            AZ_CLASS_ALLOCATOR(JoltJob, AZ::ThreadPoolAllocator);

            explicit JoltJob(JobHandle& inJobHandle, AZ::JobContext* context = nullptr);
            ~JoltJob() override = default;

        protected:
            void Process() override;
            
        private:
            JobHandle m_jobHandle;
        };

    private:
        /// Internal helper function to queue a job
        inline void	QueueJobInternal(Job *inJob);
        
        /// Array of jobs (fixed size)
        using AvailableJobs = JPH::FixedSizeFreeList<Job>;
        AvailableJobs mJobs;
        
        unsigned int m_numWorkerThreads = 0;
        
        AZStd::unique_ptr<AZ::JobManager> m_jobManager;
        AZStd::unique_ptr<AZ::JobContext> m_jobContext;
        
        /// Boolean to indicate that we want to stop the job system
        std::atomic<bool> mQuit = false;
    };
}

