# Realtime-CineFollow-Gimbal
CineFollow-Gimbal is a real-time, shooting-oriented pan–tilt gimbal designed as an intelligent camera accessory. It uses vision-based feedback to smoothly keep a subject centred in frame. The current prototype supports smartphone payloads, with future compatibility planned for mirrorless and cinema cameras via standard 1/4-inch mounts.

## **📌 Project Overview**

Real-time Requirements : This project addresses a solid real-time requirement where any delay in image processing or PID calculation would result in jerky motion or tracking loss. 

Multithreading: Separated threads for image acquisition, CV processing, and PID control using std::thread. 

Event-Driven Architecture: Utilizing Callbacks and Lambdas to handle frame-ready events, minimizing polling latency. 

Low Latency: Target loop frequency is 30-60Hz to match the camera frame rate. 

Key Features |  Modular Design: Universal 1/4" mount for cameras/smartphones.

Robust Tracking: OpenCV-based object detection with predictive motion filtering. 

Smooth Motion: Advanced PID control for professional-grade stabilization. 

Software Architecture | The project is built with Modern C++ (C++17), emphasizing encapsulation and high reliability.

GimbalController: Encapsulates PID logic and motor drive states. 

VisionProcessor: Handles frame buffers and object detection using STL containers. 

ThreadManager: Manages real-time thread priorities and safe data exchange.

## 👉 Division of responsibilities among team members

## **🚀 Development Progress**

🔄 Hardware selection & procurement ························································································✅[Completed]

🔄 Initial code framework setup (C++ & sensor drivers) ···················································✅[Completed]

🔄 Optimization of real-time data processing ·········································································✅[Completed]

🔄 Enhancing user interaction (touch buttons & LED animation) ·································✅[Completed]

🔄 Software testing & debugging ··································································································✅[Completed]

📢 Project promotion (social media & Hackaday) ·································································✅[Completed]

## **🎯 Key Features**

## **🔧 Hardware Components**

## **💻 Software Architecture**

## **📝3rd Party Components**

## **🧠 Reference & Tutorials**

## **📊 Test & Assessment**

## **📢 Future Improvements**

## **🔗 Relevant Links**

[**Documentation 📝**]()  
[**GitHub Repository 🔗**]()  
[**Demo Video 🎥**]()  
[**Social Media Promotion 📢**]()
