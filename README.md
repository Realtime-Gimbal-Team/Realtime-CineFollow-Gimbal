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

Project hardware list (current)
| Category | Item | Qty | Notes |
|---|---|---:|---|
| Compute & Control | Raspberry Pi 5 | 1 | Main controller |
| Compute & Control | Raspberry Pi 5 Active Cooler | 1 | Cooling for sustained load |
| Compute & Control | Pico 2 | 1 | Auxiliary MCU / real-time I/O (if used) |
| Vision | Raspberry Pi Camera Module 3 (Camera3) | 1 | Main camera |
| Vision | Camera ribbon cable | 1 | CSI ribbon cable |
| Actuation | GM3506 Gimbal Motor | 2 | Pitch + Yaw |
| Motor Driver | SimpleFOC mini dual driver board (1-to-2) | 1 | Drives 2 gimbal motors |
| Power | Auline 4800mAh LiPo battery | 1 | Main power source |
| Power | XT60 cable | 1 | Battery connection |
| Power | XT60 1-to-2 splitter cable | 1 | Split battery to multiple loads |
| Power | USB-C 5V buck converter module | 1 | Step-down to 5V for Raspberry Pi |
| Wiring & Connectors | Dupont jumper wires | 1 set | General wiring |
| Wiring & Connectors | XH2.54 5-pin Dupont cable | 1 | 2.54mm pitch, 5-pin |
| Mechanical | 3D-printed base | 1 | Structural mount |
| Mechanical | 3D-printed gimbal arms | 1 set | Axis arms / brackets |
| Mechanical | Phone clamp | 1 | Phone holder |
| Payload | iPhone 13 mini | 1 | Payload / demo device |
| Fasteners | M2 screws & nuts | assorted | Assembly |
| Fasteners | M2.5 screws & nuts | assorted | Assembly |

## **💻 Software Architecture**

## **📝3rd Party Components**

## **🧠 Reference & Tutorials**

## **📊 Test & Assessment**

## **📢 Future Improvements**

## **🔗 Relevant Links**

[**Documentation 📝**]()  
[**GitHub Repository 🔗**]()  
[**Demo Video 🎥**]()  
[**Social Media Promotion 📢**](https://www.tiktok.com/@embedded629?_r=1&_t=ZN-95cXb2197FG)
