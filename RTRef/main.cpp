// ======================================================================== //
// Copyright 2009-2020 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //
#include <../ext/embree/include/embree3/rtcore.h>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <limits>
#include <typeinfo>
#include <app.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/glcanvas.h>
#include <nanogui/layout.h>

#include <Eigen/Core>

const std::string bunny = "bunnyscene";
const std::string stair = "staircase";
const std::string tree = "tree";

#if defined(RTC_NAMESPACE_OPEN)
RTC_NAMESPACE_OPEN
#endif

/* -------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
  srand(time(NULL));
  string fileName = argv[1];
  string base = fileName.substr(0, fileName.find('.'));

  shared_ptr<SceneAndCam> sceneWithCam = Generator::generateScene(fileName);
  float aspect = sceneWithCam->cam.getAspect();

  int NY = 400;
  int NX = int(aspect * NY);

  nanogui::init();
  nanogui::ref<MyGUI> app = new MyGUI(base, NX, NY, sceneWithCam);
  nanogui::mainloop(16);

  rtcReleaseScene(sceneWithCam->scene);
  rtcReleaseDevice(sceneWithCam->device);

  nanogui::shutdown();
  return 0;
}