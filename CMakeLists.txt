cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE Release)
project(OpenKAI)
add_definitions("-O3 -w -Wall -c -fmessage-length=0 -MMD -MP -std=c++11")

OPTION(USE_CUDA "Use NVIDIA CUDA" ON)
OPTION(USE_OPENCV4TEGRA "Use NVIDIA OpenCV4Tegra for JetsonTX1" ON)
OPTION(USE_TENSORRT "Use NVIDIA TensorRT" ON)
OPTION(USE_SSD "Use Caffe SSD branch" OFF)
OPTION(USE_CAFFE "Use Caffe" OFF)
OPTION(USE_ZED "Use Stereolabs ZED camera" ON)

set(CUDA_include "/usr/local/cuda/include" CACHE STRING "")

set(OpenKAI_libDir /usr/lib /usr/local/lib)
set(OpenKAI_include /usr/local/include /usr/include)
set(OpenKAI_lib pthread boost_system glog)

if(USE_CUDA)
  add_definitions(-DUSE_CUDA)
  set(OpenKAI_include ${OpenKAI_include} ${CUDA_include})
  set(CUDA_ARCH "53" CACHE STRING "")
endif()

if(USE_OPENCV4TEGRA)
  add_definitions(-DUSE_OPENCV4TEGRA)
  set(OpenKAI_lib ${OpenKAI_lib} opencv_core opencv_highgui opencv_imgproc opencv_calib3d opencv_objdetect opencv_features2d opencv_gpu opencv_superres)
else()
  set(OpenCV_include "/usr/local/include" CACHE STRING "")
  set(OpenCV_lib "/usr/local/lib" CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} ${OpenCV_lib})
  set(OpenKAI_include ${OpenKAI_include} ${OpenCV_include})
  set(OpenKAI_lib ${OpenKAI_lib} opencv_core opencv_imgcodecs opencv_highgui opencv_imgproc opencv_videoio opencv_calib3d opencv_cudawarping opencv_cudaobjdetect opencv_cudaarithm opencv_cudabgsegm opencv_cudaimgproc opencv_cudaoptflow opencv_cudastereo opencv_objdetect opencv_xobjdetect opencv_tracking opencv_features2d)
endif()

if(USE_TENSORRT)
  add_definitions(-DUSE_TENSORRT)
  set(TensorRT_include_default /home/ubuntu/src/jetson-inference/build/${CMAKE_SYSTEM_PROCESSOR}/include)
  set(TensorRT_lib_default /home/ubuntu/src/jetson-inference/build/${CMAKE_SYSTEM_PROCESSOR}/lib)
  set(TensorRT_include ${TensorRT_include_default} CACHE STRING "")
  set(TensorRT_lib ${TensorRT_lib_default} CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} ${TensorRT_lib})
  set(OpenKAI_include ${OpenKAI_include} ${TensorRT_include})
  set(OpenKAI_lib ${OpenKAI_lib} jetson-inference)
endif()

if(USE_SSD)
  add_definitions(-DUSE_SSD)
  set(SSD_include "/home/ubuntu/src/caffe_ssd/caffe/include /home/ubuntu/src/caffe_ssd/caffe/build/src" CACHE STRING "")
  set(SSD_lib "/home/ubuntu/src/caffe_ssd/caffe/build/lib /home/ubuntu/src/caffe_ssd/caffe/build" CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} ${SSD_lib})
  set(OpenKAI_include ${OpenKAI_include} ${SSD_include})
  set(OpenKAI_lib ${OpenKAI_lib} caffe)
elseif(USE_CAFFE)
  add_definitions(-DUSE_CAFFE)
  set(Caffe_include "/home/ubuntu/src/caffe/include /home/ubuntu/src/caffe/build/src" CACHE STRING "")
  set(Caffe_lib "/home/ubuntu/src/caffe/build/lib /home/ubuntu/src/caffe/build" CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} ${Caffe_lib})
  set(OpenKAI_include ${OpenKAI_include} ${Caffe_include})
  set(OpenKAI_lib ${OpenKAI_lib} caffe)
endif()

if(USE_ZED)
  add_definitions(-DUSE_ZED)
  set(EIGEN3_include "/usr/include/eigen3" CACHE STRING "")
  set(ZED_include "/usr/local/zed/include" CACHE STRING "")
  set(ZED_lib "/usr/local/zed/lib" CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} ${ZED_lib})
  set(OpenKAI_include ${OpenKAI_include} ${ZED_include})
  set(OpenKAI_include ${OpenKAI_include} ${EIGEN3_include})
  set(OpenKAI_lib ${OpenKAI_lib} sl_zed)
endif()


# Setting specific lib here
link_directories(${OpenKAI_libDir})

# setup CUDA
find_package(CUDA)

set(
	CUDA_NVCC_FLAGS
	${CUDA_NVCC_FLAGS}; 
    -O3 -gencode arch=compute_${CUDA_ARCH},code=sm_${CUDA_ARCH}
)

# File
file(GLOB OpenKAI_cpp src/*.cpp src/*/*.cpp src/*/*/*.cpp src/*/*/*/*.cpp)

cuda_add_executable(OpenKAI ${OpenKAI_cpp})

# C/C++ settings
include_directories(${OpenKAI_include})
target_link_libraries(OpenKAI ${OpenKAI_lib})

