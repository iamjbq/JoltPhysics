
#pragma once

namespace JoltPhysics
{
    // System Component TypeIds
    inline constexpr const char* JoltPhysicsSystemComponentTypeId = "{32F13A17-4F2B-496E-AE84-7F9A647F53A6}";
    inline constexpr const char* JoltPhysicsEditorSystemComponentTypeId = "{4B330D5F-B174-47EB-ADDE-AEA5CBD4C7EC}";

    // Module derived classes TypeIds
    inline constexpr const char* JoltPhysicsModuleInterfaceTypeId = "{A70649D9-2749-40D4-8C3E-1ACE3B7C61E5}";
    inline constexpr const char* JoltPhysicsModuleTypeId = "{944D5DEC-6D83-439A-9063-34662363EDB0}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* JoltPhysicsEditorModuleTypeId = JoltPhysicsModuleTypeId;

    // Interface TypeIds
    inline constexpr const char* JoltPhysicsRequestsTypeId = "{8A188165-AB47-49D6-943F-1786F6792804}";
} // namespace JoltPhysics
