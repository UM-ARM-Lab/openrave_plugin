#ifndef OR_RRT_PLUGIN_H
#define OR_RRT_PLUGIN_H

#include <openrave_rrtplugin/include/openrave_rrtplugin/rrtHelper.h>

#include <atomic>
#include <thread>
#include <mutex>

#include <openrave/plugin.h>
#include <ros/ros.h>
#include <sensor_msgs/JointState.h>

using namespace OpenRAVE;

namespace my_plugin {

    class MyNewModule : public ModuleBase
    {
    public:
        MyNewModule(EnvironmentBasePtr penv, std::istream& ss) : ModuleBase(penv) {

            penv_=penv;

            RegisterCommand("MyCommand",boost::bind(&MyNewModule::MyCommand,this,_1,_2),
                            "This is an example command");

            RegisterCommand("goal",boost::bind(&MyNewModule::SetGoal,this,_1,_2),
                            "goal %f %f %f %f %f %f %f;");

            RegisterCommand("bias",boost::bind(&MyNewModule::SetBias,this,_1,_2),
                            "bias %f;");

            RegisterCommand("step",boost::bind(&MyNewModule::SetStep,this,_1,_2),
                            "step %f;");

            RegisterCommand("isBiRRT",boost::bind(&MyNewModule::SetBiRRT,this,_1,_2),
                            "isBiRRT %d;");

            RegisterCommand("SetWeight",boost::bind(&MyNewModule::SetWeight,this,_1,_2),
                            "SetWeight ;");

            RegisterCommand("SetUpperBound",boost::bind(&MyNewModule::SetUpperBound,this,_1,_2),
                            "SetUpperBound %f %f %f %f %f %f %f;");

            RegisterCommand("SetLowerBound",boost::bind(&MyNewModule::SetLowerBound,this,_1,_2),
                            "SetLowerBound %f %f %f %f %f %f %f;");

            RegisterCommand("RRT",boost::bind(&MyNewModule::RRT,this,_1,_2),
                            "Run RRT");

            RegisterCommand("BiRRT",boost::bind(&MyNewModule::BiRRT,this,_1,_2),
                            "Run BiRRT");

            RegisterCommand("Smooth",boost::bind(&MyNewModule::Smooth,this,_1,_2),
                            "Smooth %d;");

            RegisterCommand("getRRTpath",boost::bind(&MyNewModule::GetRRTPath,this,_1,_2),
                            "Path %d;");


        }
        virtual ~MyNewModule() {}

        EnvironmentBasePtr penv_;
    //    ParameterSet plannerSetup;

        configSet goalConfig;
        float goalBias = SAMPLEBIAS;
        float stepSize = STEPSIZE;
        int smooth = 0;
        int isBiRRT = 0;
        std::vector<RRTNode*> path;
    };


}


namespace armlab_or_plugins
{
    class VictorROSJointPositionController : public OpenRAVE::ControllerBase
    {
        public:
            VictorROSJointPositionController(OpenRAVE::EnvironmentBasePtr penv, std::istream& ss);

            virtual ~VictorROSJointPositionController();

            virtual bool Init(OpenRAVE::RobotBasePtr robot, const std::vector<int> &dofindices, int nControlTransformation);

            virtual const std::vector<int>& GetControlDOFIndices() const;


            virtual int IsControlTransformation() const;

            virtual OpenRAVE::RobotBasePtr GetRobot() const;

            virtual void Reset(int options = 0);

            virtual bool SetDesired(const std::vector<OpenRAVE::dReal>& values, OpenRAVE::TransformConstPtr trans = OpenRAVE::TransformConstPtr());

            virtual bool SetPath(OpenRAVE::TrajectoryBaseConstPtr ptraj);

            virtual void SimulationStep(OpenRAVE::dReal fTimeEllapsed);

            virtual bool IsDone();

            static const std::vector<std::string> left_arm_joint_names_;
            static const std::vector<std::string> right_arm_joint_names_;

            static const std::vector<std::string> left_gripper_joint_names_;
            static const std::vector<std::string> right_gripper_joint_names_;

        private:
            // OpenRAVE data
            OpenRAVE::RobotBaseWeakPtr robot_;
            std::vector<int> dof_indices_;
            std::vector<int> left_arm_dof_indices_;
            std::vector<int> right_arm_dof_indices_;

            // State machine inputs:
            std::mutex state_machine_input_mutex_;

            bool new_desired_position_;
            std::vector<OpenRAVE::dReal> desired_position_values_;
            bool new_desired_trajectory_;
            OpenRAVE::TrajectoryBaseConstPtr desired_trajectory_;

            struct InputData
            {
                public:
                    bool new_desired_position_;
                    std::vector<OpenRAVE::dReal> desired_position_values_;
                    bool new_desired_trajectory_;
                    OpenRAVE::TrajectoryBaseConstPtr desired_trajectory_;

                    std::vector<OpenRAVE::dReal> curr_joint_values_;
                    int traj_waypoint_target_ind_;
            };

            // State machine outputs
            std::atomic_bool target_reached_;

            // State machine internals
            enum STATE
            {
                INIT,
                NO_ACTIVE_TARGET,
                GOING_TO_SINGLE_POSITION,
                STARTING_NEW_TRAJ,
                FOLLOWING_TRAJ,
            };
            STATE state_machine_state_;
            int traj_waypoint_target_ind_;
            std::thread state_machine_thread_;

            // ROS Objects
            ros::NodeHandle nh_;
            ros::Subscriber joint_states_subscriber_;
            ros::Publisher right_arm_motion_command_publisher_;
            ros::Publisher left_arm_motion_command_publisher_;
            std::thread spin_thread_;


            void InitializeROSPubSub();

            std::vector<int> GetDOFIndices(const std::vector<std::string>& joint_names);

            void JointStateCallback(const sensor_msgs::JointState::ConstPtr& joint_state);


            void StateMachineLoop();

            VictorROSJointPositionController::InputData ReadInputData();

            VictorROSJointPositionController::STATE DetermineNextState(const VictorROSJointPositionController::STATE current_state, const InputData& input_data) const;

            std::vector<OpenRAVE::dReal> GetJointValuesFromTrajectory(const OpenRAVE::TrajectoryBaseConstPtr trajectory, const int waypoint_ind) const;

            static bool IsGoalReached(const std::vector<OpenRAVE::dReal>& current_vals, const std::vector<OpenRAVE::dReal>& target_vals, const std::vector<int> dof_indices_to_check);

            void SendJointCommand(const std::vector<OpenRAVE::dReal>& joint_vals);
    };
}

#endif // VICTOR_ROS_JOINT_POSITION_CONTROLLER_H
