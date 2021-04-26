////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016, Andrew Dornbush
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

/// \author Andrew Dornbush

#include <smpl/heuristic/euclid_dist_heuristic.h>

// standard includes
#include <cmath>

// project includes
#include <smpl/angles.h>
#include <smpl/console/console.h>

namespace smpl {

static const char* LOG = "heuristic.euclid_dist";

static inline
double EuclideanDistance(
    double x1, double y1, double z1,
    double x2, double y2, double z2)
{
    const double dx = x2 - x1;
    const double dy = y2 - y1;
    const double dz = z2 - z1;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool EuclidDistHeuristic::init(RobotPlanningSpace* space)
{
    if (!RobotHeuristic::init(space)) {
        return false;
    }

    m_point_ext = space->getExtension<PointProjectionExtension>();
    if (m_point_ext) {
        // SMPL_INFO_NAMED(LOG, "Got Point Projection Extension!");
    }
    m_pose_ext = space->getExtension<PoseProjectionExtension>();
    if (m_pose_ext) {
        SMPL_INFO_NAMED(LOG, "Got Pose Projection Extension!");
    }
    if (!m_pose_ext && !m_point_ext) {
        SMPL_WARN_NAMED(LOG, "EuclidDistHeuristic recommends PointProjectionExtension or PoseProjectionExtension");
    }

    return true;
}

void EuclidDistHeuristic::setWeightX(double wx)
{
    m_x_coeff = wx;
}

void EuclidDistHeuristic::setWeightY(double wy)
{
    m_y_coeff = wy;
}

void EuclidDistHeuristic::setWeightZ(double wz)
{
    m_z_coeff = wz;
}

void EuclidDistHeuristic::setWeightRot(double wr)
{
    m_rot_coeff = wr;
}

double EuclidDistHeuristic::getMetricGoalDistance(double x, double y, double z)
{
    auto& goal_pose = planningSpace()->goal().pose;
    return EuclideanDistance(
            x, y, z,
            goal_pose.translation()[0],
            goal_pose.translation()[1],
            goal_pose.translation()[2]);
}

double EuclidDistHeuristic::getMetricStartDistance(double x, double y, double z)
{
    // TODO: implement
    return 0.0;
}

Extension* EuclidDistHeuristic::getExtension(size_t class_code)
{
    if (class_code == GetClassCode<RobotHeuristic>()) {
        return this;
    }
    return nullptr;
}

int EuclidDistHeuristic::GetGoalHeuristic(int state_id)
{
    if (state_id == planningSpace()->getGoalStateID()) {
        return 0;
    }

    if (m_pose_ext) {
        Affine3 p;
        if (!m_pose_ext->projectToPose(state_id, p)) {
            return 0;
        }

        auto& goal_pose = planningSpace()->goal().pose;

        const double dist = computeDistance(p, goal_pose);

        const int h = FIXED_POINT_RATIO * dist;

        double Y, P, R;
        angles::get_euler_zyx(p.rotation(), Y, P, R);
        SMPL_DEBUG_NAMED(LOG, "h(%0.3f, %0.3f, %0.3f, %0.3f, %0.3f, %0.3f) = %d", p.translation()[0], p.translation()[1], p.translation()[2], Y, P, R, h);

        return h;
    } else if (m_point_ext) {
        Vector3 p;
        if (!m_point_ext->projectToPoint(state_id, p)) {
            return 0;
        }

        auto& goal_pose = planningSpace()->goal().pose;
        Vector3 gp(goal_pose.translation());

        double dist = computeDistance(p, gp);

        const int h = FIXED_POINT_RATIO * dist;
        SMPL_DEBUG_NAMED(LOG, "h(%d) = %d", state_id, h);
        return h;
    } else {
        return 0;
    }
}

int EuclidDistHeuristic::GetStartHeuristic(int state_id)
{
    return 0;
}

int EuclidDistHeuristic::GetFromToHeuristic(int from_id, int to_id)
{
    if (m_pose_ext) {
        if (from_id == planningSpace()->getGoalStateID()) {
            auto& gp = planningSpace()->goal().pose;
            Affine3 p;
            if (!m_pose_ext->projectToPose(to_id, p)) {
                return 0;
            }
            return (int)(FIXED_POINT_RATIO * computeDistance(gp, p));
        } else if (to_id == planningSpace()->getGoalStateID()) {
            auto& gp = planningSpace()->goal().pose;
            Affine3 p;
            if (!m_pose_ext->projectToPose(from_id, p)) {
                return 0;
            }
            return (int)(FIXED_POINT_RATIO * computeDistance(p, gp));
        } else {
            Affine3 a, b;
            if (!m_pose_ext->projectToPose(from_id, a) ||
                !m_pose_ext->projectToPose(to_id, b))
            {
                return 0;
            }
            return (int)(FIXED_POINT_RATIO * computeDistance(a, b));
        }
    } else if (m_point_ext) {
        if (from_id == planningSpace()->getGoalStateID()) {
            Vector3 gp(planningSpace()->goal().pose.translation());
            Vector3 p;
            if (!m_pose_ext->projectToPoint(to_id, p)) {
                return 0;
            }
            return (int)(FIXED_POINT_RATIO * computeDistance(gp, p));
        } else if (to_id == planningSpace()->getGoalStateID()) {
            Vector3 gp(planningSpace()->goal().pose.translation());
            Vector3 p;
            if (!m_pose_ext->projectToPoint(from_id, p)) {
                return 0;
            }
            return (int)(FIXED_POINT_RATIO * computeDistance(p, gp));
        } else {
            Vector3 a, b;
            if (!m_pose_ext->projectToPoint(from_id, a) ||
                !m_pose_ext->projectToPoint(to_id, b))
            {
                return 0;
            }
            return (int)(FIXED_POINT_RATIO * computeDistance(a, b));
        }
    } else {
        return 0;
    }
}

Affine3 EuclidDistHeuristic::createPose(
    const std::vector<double> &pose) const
{
    return createPose(pose[0], pose[1], pose[2], pose[5], pose[4], pose[3]);
}

Vector3 EuclidDistHeuristic::createPoint(
    const std::vector<double>& point) const
{
    return Vector3(point[0], point[1], point[2]);
}

Affine3 EuclidDistHeuristic::createPose(
    double x, double y, double z,
    double Y, double P, double R) const
{
    return Affine3(
            Translation3(x, y, z) *
            AngleAxis(Y, Vector3::UnitZ()) *
            AngleAxis(P, Vector3::UnitY()) *
            AngleAxis(R, Vector3::UnitX()));
}

double EuclidDistHeuristic::computeDistance(
    const Affine3& a,
    const Affine3& b) const
{
    auto sqrd = [](double d) { return d * d; };

    Vector3 diff = b.translation() - a.translation();

    double dp2 =
            m_x_coeff * sqrd(diff.x()) +
            m_y_coeff * sqrd(diff.y()) +
            m_z_coeff * sqrd(diff.z());

    Quaternion qb(b.rotation());
    Quaternion qa(a.rotation());

    double dot = qa.dot(qb);
    if (dot < 0.0) {
        qb = Quaternion(-qb.w(), -qb.x(), -qb.y(), -qb.z());
        dot = qa.dot(qb);
    }

    double dr2 = angles::normalize_angle(2.0 * std::acos(dot));
    dr2 *= (m_rot_coeff * dr2);

    SMPL_DEBUG_NAMED(LOG, "Compute Distance: sqrt(%f + %f)", dp2, dr2);

    return std::sqrt(dp2 + dr2);
}

double EuclidDistHeuristic::computeDistance(
    const Vector3& u,
    const Vector3& v) const
{
    auto sqrd = [](double d) { return d * d; };
    Vector3 diff = v - u;
    return m_x_coeff * sqrd(diff.x()) +
            m_y_coeff * sqrd(diff.y()) +
            m_z_coeff * sqrd(diff.z());
}

} // namespace smpl
