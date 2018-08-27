#include "parameters.h"

double INIT_DEPTH;
double MIN_PARALLAX;

std::vector<Eigen::Matrix3d> RIC;
std::vector<Eigen::Vector3d> TIC;

Eigen::Vector3d G{0.0, 0.0, 9.8};

double SOLVER_TIME;
int ESTIMATE_EXTRINSIC;
int ESTIMATE_TD;
double ROW, COL;
double TD, TR;
std::string EX_CALIB_RESULT_PATH;

int NUM_ITERATIONS;

template <typename T>
T readParam(ros::NodeHandle &n, std::string name)
{
    T ans;
    if (n.getParam(name, ans))
    {
        ROS_INFO_STREAM("Loaded " << name << ": " << ans);
    }
    else
    {
        ROS_ERROR_STREAM("Failed to load " << name);
        n.shutdown();
    }
    return ans;
}

void readParameters(ros::NodeHandle& n)
{
  std::string config_file;
  config_file = readParam<std::string>(n, "config_file");
  cv::FileStorage fsSettings(config_file, cv::FileStorage::READ);
  if(!fsSettings.isOpened())
  {
    std::cerr << "ERROR: Wrong path to settings" <<std::endl;
  }
  
  SOLVER_TIME = fsSettings["max_solver_time"];
  MIN_PARALLAX = fsSettings["keyframe_parallax"];
  MIN_PARALLAX = MIN_PARALLAX / FOCAL_LENGTH;
  
  ROW = fsSettings["image_height"];
  COL = fsSettings["image_width"];
  
  ROS_INFO("ROW: %f COL: %f", ROW, COL);
  
  std::string OUTPUT_PATH;
  fsSettings["out_path"] >> OUTPUT_PATH;
  
  ESTIMATE_EXTRINSIC = fsSettings["estimate_extrinsic"];
  if (ESTIMATE_EXTRINSIC == 2)
  {
      ROS_WARN("have no prior about extrinsic param, calibrate extrinsic param");
      RIC.push_back(Eigen::Matrix3d::Identity());
      TIC.push_back(Eigen::Vector3d::Zero());
      EX_CALIB_RESULT_PATH = OUTPUT_PATH + "/extrinsic_parameter.csv";

  }
  else 
  {
      if ( ESTIMATE_EXTRINSIC == 1)
      {
	  ROS_WARN(" Optimize extrinsic param around initial guess!");
	  EX_CALIB_RESULT_PATH = OUTPUT_PATH + "/extrinsic_parameter.csv";
      }
      if (ESTIMATE_EXTRINSIC == 0)
	  ROS_WARN(" fix extrinsic param ");

      cv::Mat cv_R, cv_T;
      fsSettings["extrinsicRotation"] >> cv_R;
      fsSettings["extrinsicTranslation"] >> cv_T;
      Eigen::Matrix3d eigen_R;
      Eigen::Vector3d eigen_T;
      cv::cv2eigen(cv_R, eigen_R);
      cv::cv2eigen(cv_T, eigen_T);
      Eigen::Quaterniond Q(eigen_R);
      eigen_R = Q.normalized();
      RIC.push_back(eigen_R);
      TIC.push_back(eigen_T);
      ROS_INFO_STREAM("Extrinsic_R : " << std::endl << RIC[0]);
      ROS_INFO_STREAM("Extrinsic_T : " << std::endl << TIC[0].transpose());
      
  }   
  
  INIT_DEPTH = 5.0;
  
  TD = fsSettings["td"];
  ESTIMATE_TD = fsSettings["estimate_td"];
  if (ESTIMATE_TD)
      ROS_INFO_STREAM("Unsynchronized sensors, online estimate time offset, initial td: " << TD);
  else
      ROS_INFO_STREAM("Synchronized sensors, fix time offset: " << TD);  
  
  
}

