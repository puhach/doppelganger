#ifndef RESNETFACEDESCRIPTORMETRIC_H
#define RESNETFACEDESCRIPTORMETRIC_H

#include "resnet.h"

#include "dlibmatrixdistancel2.h"

// ResNetFaceDescriptorMetric is simply an alias for L2Distance specialized for dlib::matrix 
using ResNetFaceDescriptorMetric = L2Distance<typename ResNet::Descriptor>;


#endif	// RESNETFACEDESCRIPTORMETRIC_H
