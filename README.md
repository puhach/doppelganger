# Doppelganger

This face recognition application allows the identification of a person by their picture. It currently supports face recognition using the OpenFace model and a variation of the ResNet neural network. The architecture of the program is design to be scalable and anticipate the addition of new algorithms in future.

The usage details are described below.

## Set Up

It is assumed that OpenCV 4.x, C++17 compiler, and cmake 3.0 or newer are installed on the system.

### Project structure

The project has the following directory structure:
```
│   README.md
│   
├───dataset
│           
├───dlib
│               
├───models
│       dlib_face_recognition_resnet_model_v1.dat
│       nn4.small2.v1.t7
│       nn4.v2.t7
│       shape_predictor_5_face_landmarks.dat
│       shape_predictor_68_face_landmarks.dat
│       
├───src
│   │   .gitignore
│   │   CMakeLists.txt
│   │   dlibfaceextractor.h
│   │   dlibmatrixdistancel2.h
│   │   dlibmatrixhash.h
│   │   facedb.h
│   │   facedescriptorcomputer.h
│   │   faceextractorhelper.h
│   │   labeldata.cpp
│   │   labeldata.h
│   │   main.cpp
│   │   opencvmatdistancel2.h
│   │   openface.cpp
│   │   openface.h
│   │   openfacedescriptorcomputer.h
│   │   openfacedescriptormetric.h
│   │   openfaceextractor.h
│   │   resnet.h
│   │   resnetfacedescriptorcomputer.h
│   │   resnetfacedescriptormetric.h
│   │   
│   └───build
│                           
├───test
│       readme.txt
│       shashikant-pedwal.jpg
│       sofia-solares.jpg
│       
└───writeup
        doppelganger.pdf   
        
```
