////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2011, Willow Garage, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Willow Garage, Inc. nor the names of its
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////

/// \author Benjamin Cohen

#ifndef sbpl_collision_SBPLCollisionSpace_h
#define sbpl_collision_SBPLCollisionSpace_h

// standard includes
#include <math.h>
#include <string>
#include <map>
#include <vector>

// system includes
#include <angles/angles.h>
#include <geometry_msgs/Point.h>
#include <leatherman/bresenham.h>
#include <leatherman/utils.h>
#include <moveit_msgs/CollisionObject.h>
#include <moveit_msgs/PlanningScene.h>
#include <moveit_msgs/RobotState.h>
#include <ros/ros.h>
#include <sbpl_geometry_utils/Interpolator.h>
#include <sbpl_geometry_utils/SphereEncloser.h>
#include <sbpl_geometry_utils/Voxelizer.h>
#include <sbpl_manipulation_components/collision_checker.h>
#include <sbpl_manipulation_components/occupancy_grid.h>
#include <tf_conversions/tf_kdl.h>

// project includes
#include <sbpl_collision_checking/sbpl_collision_model.h>
#include <sbpl_collision_checking/collision_model_config.h>

namespace sbpl {
namespace collision {

class SBPLCollisionSpace : public sbpl_arm_planner::CollisionChecker
{
public:

    SBPLCollisionSpace(sbpl_arm_planner::OccupancyGrid* grid);
    ~SBPLCollisionSpace();

    /// \name sbpl_arm_planner::CollisionChecker API Requirements
    ///@{

    bool isStateValid(
        const std::vector<double>& angles,
        bool verbose,
        bool visualize,
        double &dist);

    bool isStateToStateValid(
        const std::vector<double>& angles0,
        const std::vector<double>& angles1,
        int& path_length,
        int& num_checks,
        double& dist);

    bool interpolatePath(
        const std::vector<double>& start,
        const std::vector<double>& end,
        const std::vector<double>& inc,
        std::vector<std::vector<double>>& path);

    ///@}

    bool init(
        const std::string& urdf_string,
        const std::string& group_name,
        const CollisionModelConfig& config,
        const std::vector<std::string>& planning_joints);

    void setPadding(double padding);

    bool setPlanningScene(const moveit_msgs::PlanningScene& scene);

    std::string getReferenceFrame() const
    {
        return model_.getReferenceFrame(group_name_);
    }

    const std::string& getGroupName() { return group_name_; }
    void setJointPosition(const std::string& name, double position);
    bool setPlanningJoints(const std::vector<std::string>& joint_names);
    bool getCollisionSpheres(
        const std::vector<double>& angles,
        std::vector<std::vector<double>>& spheres);

    /// \name Collision Objects
    ///@{

    bool processCollisionObject(const moveit_msgs::CollisionObject& object);
    void getAllCollisionObjectVoxelPoses(
        std::vector<geometry_msgs::Pose>& poses) const;

    ///@}

    /// \name Attached Objects
    ///@{

    void attachObject(const moveit_msgs::AttachedCollisionObject& obj);
    void removeAttachedObject();

    bool getAttachedObject(
        const std::vector<double>& angles,
        std::vector<std::vector<double>>& xyz);

    ///@}

    /// \name Visualization
    ///@{

    // THE DREAM
//    visualization_msgs::MarkerArray getWorldVisualization() const; // visualization of the world
//    visualization_msgs::MarkerArray getRobotVisualization() const; // visualization of the robot
//    visualization_msgs::MarkerArray getCollisionWorldVisualization() const; // visualization of the collision world
//    visualization_msgs::MarkerArray getCollisionRobotVisualization() const; // visualization of the collision robot
//    visualization_msgs::MarkerArray getCollisionDetailsVisualization() const; // visualization of collisions between world and robot

    visualization_msgs::MarkerArray getCollisionObjectsVisualization() const;
    visualization_msgs::MarkerArray getCollisionsVisualization() const;
    visualization_msgs::MarkerArray getCollisionObjectVoxelsVisualization() const;
    visualization_msgs::MarkerArray getBoundingBoxVisualization() const;
    visualization_msgs::MarkerArray getDistanceFieldVisualization() const;
    visualization_msgs::MarkerArray getOccupiedVoxelsVisualization() const;

    visualization_msgs::MarkerArray getVisualization(const std::string& type);

    visualization_msgs::MarkerArray getCollisionModelVisualization(
        const std::vector<double>& angles);

    visualization_msgs::MarkerArray getMeshModelVisualization(
        const std::string& group_name,
        const std::vector<double>& angles);

    ///@}

    /// \name Self Collision
    ///@{

    bool updateVoxelGroups();
    bool updateVoxelGroup(Group* g);
    bool updateVoxelGroup(const std::string& name);

    ///@}

private:

    /////////////////////
    // Collision World //
    /////////////////////

    sbpl_arm_planner::OccupancyGrid* grid_;

    // set of collision objects
    std::map<std::string, moveit_msgs::CollisionObject> object_map_;

    // voxelization of objects in the grid reference frame
    std::map<std::string, std::vector<Eigen::Vector3d>> object_voxel_map_;

    /////////////////////
    // Collision Robot //
    /////////////////////

    SBPLCollisionModel model_;
    std::string group_name_;
    double padding_;
    double object_enclosing_sphere_radius_;
    std::vector<double> inc_;
    std::vector<double> min_limits_;
    std::vector<double> max_limits_;
    std::vector<bool> continuous_;
    std::vector<const Sphere*> spheres_; // temp
    std::vector<std::vector<KDL::Frame>> frames_; // temp

    //////////////////////
    // Attached Objects //
    //////////////////////

    bool object_attached_;
    int attached_object_frame_num_;
    int attached_object_segment_num_;
    int attached_object_chain_num_;
    std::string attached_object_frame_;
    std::vector<Sphere> object_spheres_;

    std::vector<Sphere> collision_spheres_;

    // return whether or not to accept an incoming collision object
    bool checkCollisionObjectAdd(const moveit_msgs::CollisionObject& object) const;
    bool checkCollisionObjectRemove(const moveit_msgs::CollisionObject& object) const;
    bool checkCollisionObjectAppend(const moveit_msgs::CollisionObject& object) const;
    bool checkCollisionObjectMove(const moveit_msgs::CollisionObject& object) const;

    bool addCollisionObject(const moveit_msgs::CollisionObject& object);
    bool removeCollisionObject(const moveit_msgs::CollisionObject& object);
    bool appendCollisionObject(const moveit_msgs::CollisionObject& object);
    bool moveCollisionObject(const moveit_msgs::CollisionObject& object);

    void removeAllCollisionObjects();

    void attachSphere(
        const std::string& name,
        const std::string& link,
        const geometry_msgs::Pose& pose,
        double radius);

    void attachCylinder(
        const std::string& link,
        const geometry_msgs::Pose& pose,
        double radius,
        double length);

    void attachCube(
        const std::string& name,
        const std::string& link,
        const geometry_msgs::Pose& pose,
        double x_dim,
        double y_dim,
        double z_dim);

    void attachMesh(
        const std::string& name,
        const std::string& link,
        const geometry_msgs::Pose& pose,
        const std::vector<geometry_msgs::Point>& vertices,
        const std::vector<int>& triangles);

    std::vector<int> convertToVertexIndices(
        const std::vector<shape_msgs::MeshTriangle>& triangles) const;

    bool checkCollision(
        const std::vector<double>& angles,
        bool verbose,
        bool visualize,
        double& dist);

    bool checkPathForCollision(
        const std::vector<double>& start,
        const std::vector<double>& end,
        bool verbose,
        int& path_length,
        int& num_checks,
        double& dist);

    inline bool isValidCell(
        const int x, const int y, const int z, const int radius);

    double isValidLineSegment(
        const std::vector<int> a, const std::vector<int> b, const int radius);

    bool getClearance(
        const std::vector<double>& angles,
        int num_spheres,
        double& avg_dist,
        double& min_dist);

    bool voxelizeCollisionObject(const moveit_msgs::CollisionObject& object);

    bool voxelizeBox(
        const shape_msgs::SolidPrimitive& box,
        const geometry_msgs::Pose& pose,
        std::vector<Eigen::Vector3d>& voxels);
    bool voxelizeSphere(
        const shape_msgs::SolidPrimitive& sphere,
        const geometry_msgs::Pose& pose,
        std::vector<Eigen::Vector3d>& voxels);
    bool voxelizeCylinder(
        const shape_msgs::SolidPrimitive& cylinder,
        const geometry_msgs::Pose& pose,
        std::vector<Eigen::Vector3d>& voxels);
    bool voxelizeCone(
        const shape_msgs::SolidPrimitive& cone,
        const geometry_msgs::Pose& pose,
        std::vector<Eigen::Vector3d>& voxels);
    bool voxelizeMesh(
        const shape_msgs::Mesh& mesh,
        const geometry_msgs::Pose& pose,
        std::vector<Eigen::Vector3d>& voxels);
};

inline bool SBPLCollisionSpace::isValidCell(
    const int x,
    const int y,
    const int z,
    const int radius)
{
    if (grid_->getCell(x,y,z) <= radius) {
        return false;
    }
    return true;
}

} // namespace collision
} // namespace sbpl 

#endif

