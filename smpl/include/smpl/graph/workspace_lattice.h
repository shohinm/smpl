////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2011, Benjamin Cohen, Andrew Dornbush
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     1. Redistributions of source code must retain the above copyright notice
//        this list of conditions and the following disclaimer.
//     2. Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in the
//        documentation and/or other materials provided with the distribution.
//     3. Neither the name of the copyright holder nor the names of its
//        contributors may be used to endorse or promote products derived from
//        this software without specific prior written permission.
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
/// \author Andrew Dornbush

#ifndef sbpl_manip_workspace_lattice_h
#define sbpl_manip_workspace_lattice_h

// standard includes
#include <vector>

// system includes
#include <ros/ros.h>
#include <sbpl/headers.h>

// project includes
#include <smpl/collision_checker.h>
#include <smpl/occupancy_grid.h>
#include <smpl/planning_params.h>
#include <smpl/robot_model.h>
#include <smpl/types.h>
#include <smpl/graph/motion_primitive.h>
#include <smpl/graph/robot_planning_space.h>
#include <smpl/graph/workspace_lattice_base.h>

namespace sbpl {
namespace motion {

#define BROKEN 1

struct WorkspaceLatticeState
{
    WorkspaceCoord coord;
    RobotState state;
    int h;
};

std::ostream& operator<<(std::ostream& o, const WorkspaceLatticeState& s);

inline
bool operator==(const WorkspaceLatticeState& a, const WorkspaceLatticeState& b)
{
    return a.coord == b.coord;
}

} // namespace motion
} // namespace sbpl

namespace std {

template <>
struct hash<sbpl::motion::WorkspaceLatticeState>
{
    typedef sbpl::motion::WorkspaceLatticeState argument_type;
    typedef std::size_t result_type;
    result_type operator()(const argument_type& s) const;
};

} // namespace std

namespace sbpl {
namespace motion {

/// \class Discrete state lattice representation representing a robot as the
///     pose of one of its links and all redundant joint variables
class WorkspaceLattice : public WorkspaceLatticeBase
{
public:

    struct PoseGoal
    {
        SixPose pose;
        Position offset;
        SixPose tolerance;
    };

    WorkspaceLattice(
        RobotModel* robot,
        CollisionChecker* checker,
        PlanningParams* params,
        OccupancyGrid* grid);

    ~WorkspaceLattice();

    bool init(const Params& params) override;
    bool initialized() const override;

    // TODO: add path extraction function that returns path in workspace rep

    /// \name Reimplemented Public Functions from RobotPlanningSpace
    ///@{
    virtual int GetGoalHeuristic(int state_id) override;
    ///@}

    /// \name Required Public Functions from RobotPlanningSpace
    ///@{
    bool setStart(const RobotState& state);
    bool setGoal(const GoalConstraint& goal);

    int getStartStateID() const;
    int getGoalStateID() const;

    bool extractPath(
        const std::vector<int>& ids,
        std::vector<RobotState>& path);
    ///@}

    /// \name Required Public Functions from Extension
    ///@{
    virtual Extension* getExtension(size_t class_code);
    ///@}

    /// \name Required Public Functions from DiscreteSpaceInformation
    ///@{
    virtual void GetSuccs(
        int state_id,
        std::vector<int>* succs,
        std::vector<int>* costs) override;

    virtual void GetPreds(
        int state_id,
        std::vector<int>* preds,
        std::vector<int>* costs) override;

    virtual void PrintState(
        int state_id,
        bool verbose,
        FILE* fout = nullptr) override;
    ///@}

private:

    WorkspaceLatticeState* m_goal_entry;
    int m_goal_state_id;

    WorkspaceLatticeState* m_start_entry;
    int m_start_state_id;

    // maps state -> id
    typedef WorkspaceLatticeState StateKey;
    typedef PointerValueHash<StateKey> StateHash;
    typedef PointerValueEqual<StateKey> StateEqual;
    hash_map<StateKey*, int, StateHash, StateEqual> m_state_to_id;

    // maps id -> state
    std::vector<WorkspaceLatticeState*> m_states;

    clock_t m_t_start;
    bool m_near_goal;

    int m_goal_coord[6];

    /// \name State Space Configuration
    ///@{

    ///@}

    /// \name Action Space Configuration
    ///@{

    std::vector<MotionPrimitive> m_prims;

    ///@}

    bool setGoalPose(const GoalConstraint& goal);
    bool setGoalPoses(const std::vector<PoseGoal>& goals);

    int createState(const WorkspaceCoord& coord);
    WorkspaceLatticeState* getState(int state_id);

    void getActions(const WorkspaceLatticeState& state, std::vector<Action>& actions);

    bool checkAction(
        const RobotState& state,
        const Action& action,
        double& dist,
        RobotState* final_rstate = nullptr);

    bool isGoal(const WorkspaceState& state);

    visualization_msgs::MarkerArray getStateVisualization(
        const RobotState& state,
        const std::string& ns);

#if !BROKEN
    std::vector<double> mp_gradient_;
    std::vector<double> mp_dist_;

    bool getMotionPrimitive(WorkspaceLatticeState* parent, MotionPrimitive& mp);
    void getAdaptiveMotionPrim(
        int type,
        WorkspaceLatticeState* parent,
        MotionPrimitive& mp);
    void getVector(
        int x1, int y1, int z1,
        int x2, int y2, int z2,
        int& xout, int& yout, int& zout,
        int multiplier,
        bool snap = true);
    bool getDistanceGradient(int& x, int& y, int& z);
#endif
};

} // namespace motion
} // namespace sbpl

#endif

