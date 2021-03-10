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

### Dataset

The dataset directory is supposed to contain subfolders whose names correspond to labels. Image files inside each subfolder are considered to have a label of the folder. 

The project was tested on a subset of the Celebrity Together Dataset containing 5 face images of around 1100 celebrities.

```
├───dataset
│   │   readme.txt
│   │   
│   ├───n00000001
│   │       n00000001_00000263.JPEG
│   │       n00000001_00000405.JPEG
│   │       n00000001_00000412.JPEG
│   │       n00000001_00000583.JPEG
│   │       n00000001_00000900.JPEG
│   │       
│   ├───n00000003
│   │       n00000003_00000386.JPEG
│   │       n00000003_00000488.JPEG
│   │       n00000003_00000514.JPEG
│   │       n00000003_00000670.JPEG
│   │       n00000003_00000922.JPEG
│          
│   ...
│   │       
│   │       
│   │       
│   └───n00002619
│           n00002619_00000265.JPEG
│           n00002619_00000394.JPEG
│           n00002619_00000414.JPEG
│           n00002619_00000497.JPEG
│           n00002619_00000506.JPEG

```
