#ifndef RESNETFACEDESCRIPTORMETRIC_H
#define RESNETFACEDESCRIPTORMETRIC_H

#include "resnet.h"
#include "dlibmatrixdistancel2.h"
//#include "dlibmatrixl2distance.h"
//#include "l2distance.h"
//#include "l2distancedlibmatrix.h"

//static_assert(std::is_same<typename ResNet::OutputLabel, dlib::matrix<>>);
using ResNetFaceDescriptorMetric = L2Distance<typename ResNet::OutputLabel>;

#endif	// RESNETFACEDESCRIPTORMETRIC_H