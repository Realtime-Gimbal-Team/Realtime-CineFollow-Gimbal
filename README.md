# Realtime-CineFollow-Gimbal
CineFollow-Gimbal is a real-time, shooting-oriented pan–tilt gimbal designed as an intelligent camera accessory. It uses vision-based feedback to smoothly keep a subject centred in frame. The current prototype supports smartphone payloads, with future compatibility planned for mirrorless and cinema cameras via standard 1/4-inch mounts.
<h1 align="center">IRIS</h1>

<p align="center">
  Real-time Intelligent Cine Follow Gimbal
</p>

<p align="center">
  <img src="https://raw.githubusercontent.com/Realtime-Gimbal-Team/Realtime-CineFollow-Gimbal/main/iris.png" width="800"/>
</p>

## **📌 Project Overview**
## 📸 Prototype

<p align="center">
  <img src="https://raw.githubusercontent.com/Realtime-Gimbal-Team/Realtime-CineFollow-Gimbal/main/project.png" width="700"/>
</p>

<p align="center">
  Physical prototype of the IRIS real-time cine-follow gimbal system.
</p>
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
- **Yining Liu(3153782Y)**:Led system decision-making, visual algorithm development, and attitude sensing; implemented YOLO + ByteTrack deployment, IMU sensor fusion, gimbal control logic, and latency-oriented performance evaluation.
- **Zongwei Xie (3085969X)**:Developed the low-level motor control and hardware execution layer; implemented the motor driver based on SimpleFOC, tuned PID control loops, and handled real-time UART communication and protocol parsing.
- **Yifei Wang (3147822W)**: Designed and implemented user interaction and physical data visualization; developed OLED display functions, button-based mode switching, and event-driven interaction logic using interrupts or callbacks.
- **Yandong Fan (3159430F)**: Took charge of reliability engineering and latency assessment; built quantitative timing tools, supported unit testing for core modules, and carried out memory and fault management checks.
- **Chenxu Li (3091645L)**:Managed project coordination, revision control, and external promotion; defined Git branching and issue-tracking workflows, prepared reproducible project documentation, and supported public outreach on social media.

## **🚀 Development Progress**

🔄 Hardware selection & procurement ························································································✅[Completed]

🔄 Initial code framework setup (C++ & sensor drivers) ···················································✅[Completed]

🔄 Optimization of real-time data processing ·········································································✅[Completed]

🔄 Enhancing user interaction  ·································✅[Completed]

🔄 Software testing & debugging ··································································································✅[Completed]

📢 Project promotion (social media & Hackaday) ·································································✅[Completed]

## **🎯 Key Features**
✅ Real-time Object Tracking (YOLO + ByteTrack)
A deep learning–based vision pipeline is deployed on Raspberry Pi to perform real-time object detection and identity tracking, providing stable target coordinates for control.

✅ Gimbal Attitude Estimation (IMU Sensor Fusion)
IMU data is fused to estimate the gimbal’s orientation (Yaw/Pitch) with smooth and stable outputs, improving overall system accuracy and robustness.

✅ Smooth Motion Control (Gimbal Logic)
Camera motion principles are translated into control algorithms, incorporating S-curve smoothing, deadzone filtering, and target motion prediction for natural and cinematic movement.

✅ High-Precision Motor Control (FOC + PID)
Brushless motors are driven using the SimpleFOC framework, with well-tuned position and velocity PID loops to achieve precise and low-vibration actuation.

✅ Low-Latency Communication Architecture
Efficient UART communication combined with interrupt/callback mechanisms ensures fast and reliable transmission between the vision module and motor controller.

✅ OLED-Based Real-time Visualization & Interaction
A 0.96" OLED display provides real-time feedback including orientation angles, system runtime, and detection confidence, along with button-based mode switching.

✅ Quantitative Latency Assessment
A dedicated C++ tool measures the full pipeline delay (from vision detection to motor response) with microsecond-level precision, ensuring real-time performance.

✅ Robustness & Reliability Engineering
Unit testing (Google Test) and memory analysis (Valgrind) are applied to ensure system stability, safety, and fail-safe operation.
## **🔧 Hardware Components**
| Component                        | Specification                   | Quantity | Purpose                                                                 |
| -------------------------------- | ------------------------------- | -------: | ----------------------------------------------------------------------- |
| Raspberry Pi Pico 2              | Microcontroller board           |        1 | Low-level motor control and embedded task execution                     |
| Raspberry Pi 5                   | Single-board computer           |        1 | Main processing unit for vision, control logic, and system coordination |
| Raspberry Pi 5 Active Cooler     | Official active cooling module  |        1 | Thermal management for stable Raspberry Pi 5 operation                  |
| GM3506 Gimbal Motor              | Brushless gimbal motor          |        2 | Two-axis gimbal actuation for yaw and pitch movement                    |
| Auline 4800mAh LiPo Battery      | Rechargeable LiPo battery       |        1 | Portable power supply for the whole system                              |
| XT60 One-to-Two Cable            | XT60 splitter cable             |        1 | Power distribution from battery to multiple modules                     |
| XT60 Cable                       | Standard XT60 power cable       |        1 | Main battery connection and power transmission                          |
| USB-C 5V Buck Converter Module   | DC step-down module             |        1 | Converts battery voltage to stable 5V supply for electronics            |
| SimpleFOC Mini Dual Driver Board | One-driver-for-two-motors board |        1 | Drives and controls the two gimbal motors                               |
| Raspberry Pi Camera Module 3     | Official camera module          |        1 | Visual input for object detection and tracking                          |
| Camera Ribbon Cable              | CSI camera cable                |        1 | Connects Camera Module 3 to Raspberry Pi 5                              |
| Dupont Wires                     | Various jumper wires            |        - | General signal and power wiring between modules                         |
| 3D Printed Base                  | Custom structural part          |        1 | Mechanical support for mounting the system components                   |
| 3D Printed Gimbal Arm            | Custom axis arm structure       |    1 set | Supports motor installation and gimbal movement                         |
| XH2.54 5-Pin Cable               | 5-pin Dupont/JST-style cable    |        1 | Electrical connection between driver board and motors/sensors           |
| Phone Clamp                      | Adjustable holder               |        1 | Holds the mobile phone securely on the gimbal platform                  |
| iPhone 13 mini                   | Smartphone payload              |        1 | Target payload for stabilization and tracking demonstration             |
| M2 / M2.5 Screws and Nuts        | Fasteners                       | Assorted | Mechanical assembly and structural fixing                               |

## **💻 Software Architecture**


## **📝3rd Party Components**

## **🧠 Reference & Tutorials**

## **📊 Test & Assessment**

## **📢 Future Improvements**

## **🔗 Relevant Links**

[**Documentation 📝**]()  
[**GitHub Repository 🔗**](https://github.com/Realtime-Gimbal-Team/Realtime-CineFollow-Gimbal)  
[**Demo Video 🎥**]()  
[**Social Media Promotion 📢**](https://www.tiktok.com/@embedded629?_r=1&_t=ZN-95cXb2197FG)
