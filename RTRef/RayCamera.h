#include <RTUtil/Camera.hpp>
#include <Eigen/Core>
#include <ext/embree/include/embree3/rtcore.h>
#include <stdio.h>

class RayCamera
{

  // The width-to-height aspect ratio
  float aspectRatio;

  // The location of the camera
  Eigen::Vector3f eye;
  // The looking direction
  Eigen::Vector3f lookDir;
  // The up direction
  Eigen::Vector3f upDir;

  // The camera basis v
  Eigen::Vector3f vv;
  // The camera basis u
  Eigen::Vector3f vu;
  // The camera basis w
  Eigen::Vector3f vw;

  // The horizontal field of view
  float hfov;

  // The near and far planes
  float near;
  float far;

public:
  /// Initalizes a default Camera.
  RayCamera();

  /// Initalizes a custom Camera.
  /// @param position The starting location of the Camera.
  /// @param target The point in the center of the Camera's focus.
  /// @param up The direction opposite of gravity.
  /// @param hfov The horizontal field of view in radians.
  /// @param aspectRatio The aspect ratio of the Camera (width/height).
  /// @param near The distance to the Camera's near plane.
  /// @param far The distance to the Camera's far plane.
  RayCamera(
      Eigen::Vector3f position,
      Eigen::Vector3f target,
      Eigen::Vector3f up,
      float hfov,
      float aspectRatio,
      float near,
      float far);

  // Generate a ray passing through (u,v) (normalized coords)
  RTCRay generateRay(float u, float v);

  void orbit(float theta, float phi);

  Eigen::Vector3f getvw();

  void zoom(float z);

  float getAspect();
};
