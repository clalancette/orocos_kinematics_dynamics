// Copyright  (C)  2009, 2011

// Version: 1.0
// Author: Ruben Smits, Herman Bruyninckx, Azamat Shakhimardanov
// Maintainer: Ruben Smits, Azamat Shakhimardanov
// URL: http://www.orocos.org/kdl

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef KDL_CHAINIDSOLVER_VERESHCHAGIN_HPP
#define KDL_CHAINIDSOLVER_VERESHCHAGIN_HPP

#include "chainidsolver.hpp"
#include "frames.hpp"
#include "articulatedbodyinertia.hpp"

#include<Eigen/StdVector>

namespace KDL
{
/**
 * \brief Dynamics calculations by constraints based on Vereshchagin 1989.
 * for a chain. This class creates instance of hybrid dynamics solver.
 * The solver calculates total joint space accelerations in a chain when a constraint force(s) is applied
 * to the chain's end-effector (task space/cartesian space).
 * For more details on this solver, see the documentation in "chainhdsolver_vereshchagin_doc.md".
 */

class ChainHdSolver_Vereshchagin : KDL::SolverI
{
    typedef std::vector<Twist> Twists;
    typedef std::vector<Frame> Frames;
    typedef Eigen::Matrix<double, 6, 1 > Vector6d;
    typedef Eigen::Matrix<double, 6, 6 > Matrix6d;
    typedef Eigen::Matrix<double, 6, Eigen::Dynamic> Matrix6Xd;

public:
    /**
     * Constructor for the solver, it will allocate all the necessary memory
     * \param chain The kinematic chain to calculate the inverse dynamics for, an internal copy will be made.
     * \param root_acc The acceleration vector of the root to use during the calculation.(most likely contains gravity)
     *
     */
    ChainHdSolver_Vereshchagin(const Chain& chain, Twist root_acc, unsigned int nc);

    ~ChainHdSolver_Vereshchagin()
    {
    };

    /**
     * This method calculates joint space constraint torques and total joint space acceleration.
     * It returns 0 when it succeeds, otherwise -1 or -2 for unmatching matrix and array sizes.
     * Input parameters;
     * \param q The current joint positions
     * \param q_dot The current joint velocities
     * \param f_ext The external forces (no gravity, it is given in root acceleration) on the segments.
     * Output parameters:
     * \param q_dotdot The joint accelerations
     * \param torques the resulting constraint torques for the joints
     *
     * @return error/success code
     */
    int CartToJnt(const JntArray &q, const JntArray &q_dot, JntArray &q_dotdot, const Jacobian& alfa, const JntArray& beta, const Wrenches& f_ext, JntArray &torques);

    /// @copydoc KDL::SolverI::updateInternalDataStructures
    virtual void updateInternalDataStructures();

    //Returns cartesian acceleration of links in base coordinates
    void getTransformedLinkAcceleration(Twists& x_dotdot);

    /*
    //Returns cartesian positions of links in base coordinates
    void getLinkCartesianPose(Frames& x_base);
    //Returns cartesian velocities of links in base coordinates
    void getLinkCartesianVelocity(Twists& xDot_base);
    //Returns cartesian acceleration of links in base coordinates
    void getLinkCartesianAcceleration(Twists& xDotDot_base);
    //Returns cartesian positions of links in link tip coordinates
    void getLinkPose(Frames& x_local);
    //Returns cartesian velocities of links in link tip coordinates
    void getLinkVelocity(Twists& xDot_local);
    //Returns cartesian acceleration of links in link tip coordinates
    void getLinkAcceleration(Twists& xDotdot_local);
    //Acceleration energy due to unit constraint forces at the end-effector
    void getLinkUnitForceAccelerationEnergy(Eigen::MatrixXd& M);
    //Acceleration energy due to arm configuration: bias force plus input joint torques
    void getLinkBiasForceAcceleratoinEnergy(Eigen::VectorXd& G);

    void getLinkUnitForceMatrix(Matrix6Xd& E_tilde);

    void getLinkBiasForceMatrix(Wrenches& R_tilde);

    void getJointBiasAcceleration(JntArray &bias_q_dotdot);
    */
private:
    /**
     *  This method calculates all cartesian space poses, twists, bias accelerations.
     *  External forces are also taken into account in this outward sweep.
     */
    void initial_upwards_sweep(const JntArray &q, const JntArray &q_dot, const JntArray &q_dotdot, const Wrenches& f_ext);
    /**
     *  This method is a force balance sweep. It calculates articulated body inertias and bias forces.
     *  Additionally, acceleration energies generated by bias forces and unit forces are calculated here.
     */
    void downwards_sweep(const Jacobian& alfa, const JntArray& torques);
    /**
     *  This method calculates constraint force magnitudes.
     *
     */
    void constraint_calculation(const JntArray& beta);
    /**
     *  This method puts all acceleration contributions (constraint, bias, nullspace and parent accelerations) together.
     *
     */
    void final_upwards_sweep(JntArray &q_dotdot, JntArray &torques);

private:
    const Chain& chain;
    unsigned int nj;
    unsigned int ns;
    unsigned int nc;
    Twist acc_root;
    Jacobian alfa_N;
    Jacobian alfa_N2;
    Eigen::MatrixXd M_0_inverse;
    Eigen::MatrixXd Um;
    Eigen::MatrixXd Vm;
    JntArray beta_N;
    Eigen::VectorXd nu;
    Eigen::VectorXd nu_sum;
    Eigen::VectorXd Sm;
    Eigen::VectorXd tmpm;
    Wrench qdotdot_sum;
    Frame F_total;

    struct segment_info
    {
        Frame F; //local pose with respect to previous link in segments coordinates
        Frame F_base; // pose of a segment in root coordinates
        Twist Z; //Unit twist
        Twist v; //twist
        Twist acc; //acceleration twist
        Wrench U; //wrench p of the bias forces (in cartesian space)
        Wrench R; //wrench p of the bias forces
        Wrench R_tilde; //vector of wrench p of the bias forces (new) in matrix form
        Twist C; //constraint
        Twist A; //constraint
        ArticulatedBodyInertia H; //I (expressed in 6*6 matrix)
        ArticulatedBodyInertia P; //I (expressed in 6*6 matrix)
        ArticulatedBodyInertia P_tilde; //I (expressed in 6*6 matrix)
        Wrench PZ; //vector U[i] = I_A[i]*S[i]
        Wrench PC; //vector E[i] = I_A[i]*c[i]
        double D; //vector D[i] = S[i]^T*U[i]
        Matrix6Xd E; //matrix with virtual unit constraint force due to acceleration constraints
        Matrix6Xd E_tilde;
        Eigen::MatrixXd M; //acceleration energy already generated at link i
        Eigen::VectorXd G; //magnitude of the constraint forces already generated at link i
        Eigen::VectorXd EZ; //K[i] = Etiltde'*Z
        double nullspaceAccComp; //Azamat: constribution of joint space u[i] forces to joint space acceleration
        double constAccComp; //Azamat: constribution of joint space constraint forces to joint space acceleration
        double biasAccComp; //Azamat: constribution of joint space bias forces to joint space acceleration
        double totalBias; //Azamat: R+PC (centrepital+coriolis) in joint subspace
        double u; //vector u[i] = torques(i) - S[i]^T*(p_A[i] + I_A[i]*C[i]) in joint subspace. Azamat: In code u[i] = torques(i) - s[i].totalBias

        segment_info(unsigned int nc):
            D(0),nullspaceAccComp(0),constAccComp(0),biasAccComp(0),totalBias(0),u(0)
        {
            E.resize(6, nc);
            E_tilde.resize(6, nc);
            G.resize(nc);
            M.resize(nc, nc);
            EZ.resize(nc);
            E.setZero();
            E_tilde.setZero();
            M.setZero();
            G.setZero();
            EZ.setZero();
        };
    };

    std::vector<segment_info, Eigen::aligned_allocator<segment_info> > results;

};
}

#endif
