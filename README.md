# Realtime-CineFollow-Gimbal

<div align="center">
  <img src="assets/images/iris.png" width="800"/>
  <h1>👁️ IRIS</h1>
  <p><b>Real-time Intelligent Cine-Follow Gimbal</b></p>
  <p><i>The autonomous, real-time camera operator for solo content creators.</i></p>

  <p>
    <a href="https://realtime-gimbal-team.github.io/CineFollow-Docs/">
      <img src="https://img.shields.io/badge/Docs-Project_Manual-blueviolet?style=for-the-badge&logo=gitbook" alt="Documentation">
    </a>
    <a href="./docs/Final_Technical_Report.pdf">
      <img src="https://img.shields.io/badge/Report-Technical_Paper-green?style=for-the-badge&logo=read-the-docs" alt="Technical Report">
    </a>
    <a href="INSERT_YOUTUBE_LINK_HERE">
      <img src="https://img.shields.io/badge/Demo_Video-Watch_Now-red?style=for-the-badge&logo=youtube" alt="Demo Video">
    </a>
    <a href="https://github.com/Realtime-Gimbal-Team/Realtime-CineFollow-Gimbal/releases/tag/v1.0.0-Final">
      <img src="https://img.shields.io/badge/Release-v1.0.0--Final-blue?style=for-the-badge&logo=github" alt="Release">
    </a>
  </p>

  <p>
    <a href="./assets/3d_models/">
      <img src="https://img.shields.io/badge/Hardware-3D_Models-orange?style=flat-square&logo=autodesk" alt="3D Models">
    </a>
    <a href="https://www.tiktok.com/@embedded629">
      <img src="https://img.shields.io/badge/TikTok-Follow_Us-black?style=flat-square&logo=tiktok" alt="Social Media">
    </a>
  </p>
</div>

---

### 📖 Quick Navigation
> **Project for ENG 5220: Real Time Embedded Programming @ University of Glasgow**
> 
> [🌐 Online Manual](https://realtime-gimbal-team.github.io/CineFollow-Docs/) | [📄 Read Final Technical Report](./docs/) | [📦 Download 3D Print Models](./assets/3d_models/) | [🎬 Demo Video (YouTube)](INSERT_YOUTUBE_LINK_HERE) | [🚀 Latest Release](https://github.com/Realtime-Gimbal-Team/Realtime-CineFollow-Gimbal/releases/tag/v1.0.0-Final)

---
---

## **📌 Project Overview**

**IRIS** is a high-performance, dual-axis cine-follow gimbal engineered to solve the "solo shooter" problem. By offloading complex tracking and stabilization tasks to an autonomous embedded system, IRIS allows creators to remain the focus of their story without needing a human camera operator.

The project is built on the **Raspberry Pi 5** platform, pushing the boundaries of real-time computer vision and motion control in a mobile form factor.

### 🛠️ Core Engineering Philosophy

1.  **Low-Level Camera Integration (`libcamera`)**: 
    We implemented a high-efficiency camera interface by **strictly adhering to the standard `libcamera` framework** recommended for the Raspberry Pi 5. By encapsulating the professor's prescribed C++ interface logic, we achieved seamless V4L2-level buffer management, ensuring that the OpenCV vision node receives raw frames with minimal kernel-to-userspace overhead.
2.  **Deterministic Control Logic**: 
    The system operates on a **100Hz deterministic control loop**. By utilizing `std::this_thread::sleep_until` and monotonic clocks, we ensure the Yaw and Pitch motors receive velocity updates at precise intervals, eliminating jitter caused by scheduling variances.
3.  **Event-Driven Architecture**: 
    Instead of wasteful polling, the IRIS software stack is entirely **event-driven**. Image acquisition triggers asynchronous callbacks, and motor commands are dispatched via non-blocking UART protocols, ensuring the CPU remains responsive to high-priority safety interrupts.
4.  **Hardware-Software Synergy**: 
    From custom 3D-printed axis arms to the SimpleFOC brushless drive implementation, every component is tuned to minimize mechanical resonance and maximize the tracking bandwidth of the Vision Node.

---
## 📸 Prototype

<p align="center">
  <img src="assets/images/prototype.png" width="700"/>
</p>

<p align="center">
  <i>Physical prototype of the IRIS real-time cine-follow gimbal system.</i>
</p>

### ⚡ Core Real-Time Requirements & Architecture
- **Strict Real-Time Execution:** This project addresses a solid real-time requirement where any delay in image processing or PID calculation would result in jerky motion or tracking loss. We achieved a **deterministic 100Hz control loop** utilizing `std::this_thread::sleep_until`, completely avoiding blocking polling statements.
- **Event-Driven Architecture:** Utilizes Callbacks and Lambdas to handle frame-ready events (`camera.startCapture`), significantly minimizing polling latency.
- **Multithreading & Memory Safety:** Separated threads for vision acquisition, CV processing, and PID control using `std::thread`. Shared states are strictly protected by `std::mutex`. The system relies entirely on modern C++ (C++17) RAII for failsafe memory management with zero explicit `new/delete` operations.

## System Architecture & Workflow

The Iris Gimbal operates on a high-concurrency, multi-threaded architecture designed to bridge real-time AI perception with fluid mechanical actuation. The system is structured into three primary decoupled threads to ensure ultra-low latency and deterministic control.

<p align="center">
  <img src="assets/images/system_flowchart.png" alt="System Flowchart" width="800"/>
<p align="center">
  <img src="assets/images/system_flowchart2.png" alt="System Flowchart" width="300"/>
</p>

### Thread Logic Breakdown:

* **Vision Perception Thread**: Executes YOLOv8-Pose inference (approx. 86ms) on the Raspberry Pi 5. It handles anatomical keypoint anchoring (Nose-point tracking) and manages the **1.5s gesture state machine** for non-contact interaction.
* **Kinematic Control Thread**: Consumes visual error data to calculate target velocities. It utilizes a **Symmetric Step Smoothing** algorithm (step = 0.015) to eliminate high-frequency jitter and ensure cinematic motion profiles.
* **UART Communication Thread**: Encapsulates velocity commands into the transmission protocol and dispatches them to the motor driver at a consistent 50Hz frequency.

## 👉 Division of responsibilities among team members
- **Yining Liu (3153782Y)**: Led system decision-making, visual algorithm development, and core C++ architecture; implemented YOLO deployment, Greedy Distance Matching tracking, Symmetric Soft Ramp control logic, Mutex Threading Architecture, and latency-oriented performance evaluation.
- **Zongwei Xie (3085969X)**: Developed the low-level motor control and hardware execution layer; implemented the motor driver based on SimpleFOC, tuned PID control loops, and handled real-time UART communication and protocol parsing.
- **Yifei Wang (3147822W)**: Responsible for system integration and runtime logic; implemented mode switching and event-driven control, and assisted in debugging and improving system stability.
- **Yandong Fan (3159430F)**: Took charge of reliability engineering and latency assessment; built quantitative timing tools, supported unit testing for core modules, and carried out memory and fault management checks.
- **Chenxu Li (3091645L)**: Managed project coordination, revision control, and external promotion; defined Git branching and issue-tracking workflows, prepared reproducible project documentation, and supported public outreach on social media.

## **🚀 Development Progress**

🔄 Hardware selection & procurement ·····························································································································✅[Completed]

🔄 Initial code framework setup (C++ & sensor drivers) ···································································································✅[Completed]

🔄 Optimization of real-time data processing ····················································································································✅[Completed]

🔄 Enhancing user interaction  ··············································································································································✅[Completed]

🔄 Software testing & debugging ····································································································································✅[Completed]

📢 Project promotion  ·························································································································································✅[Completed]

## **🎯 Key Features**
✅ **Real-time Object Tracking (YOLOv8 + Greedy Distance Matching)**
A deep learning–based vision pipeline is deployed on Raspberry Pi via NCNN. We utilized an optimized Greedy Distance Matching algorithm to provide stable target coordinates and ID persistence without the phase lag introduced by traditional filters.

✅ **Gimbal Attitude Estimation (IMU Sensor Fusion)**
IMU data is fused to estimate the gimbal’s orientation (Yaw/Pitch) with smooth and stable outputs, improving overall system accuracy and robustness.

✅ **Smooth Motion Control (Symmetric Soft Ramp & Deadzone)**
Camera motion principles are translated into control algorithms, incorporating Symmetric Linear Soft Ramps and Continuous Soft Deadzone Compensation to eliminate low-speed cogging torque and provide natural, cinematic movement.

✅ **High-Precision Motor Control (FOC + PID)**
Brushless motors are driven using the SimpleFOC framework, with well-tuned position and velocity PID loops to achieve precise and low-vibration actuation.

✅ **Low-Latency Communication Architecture**
Efficient UART communication combined with interrupt/callback mechanisms ensures fast and reliable transmission between the vision module and motor controller.

✅ **OLED-Based Real-time Visualization & Interaction**
A 0.96" OLED display provides real-time feedback including orientation angles, system runtime, and detection confidence, along with button-based mode switching.

✅ **Quantitative Latency Assessment**
A dedicated C++ tool measures the full pipeline delay (from vision detection to motor response) with microsecond-level precision, ensuring real-time performance.

✅ **Robustness & Reliability Engineering**
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

### 🔧 3D Printed Structural Assets
All custom mechanical components are available in the `/assets/3d_models/` directory for reproduction:
* `iris_base_mount.stl`: Primary mounting stand.
* `iris_yaw_arm_l_shape.stl`: Main Yaw-axis structural arm.
* `iris_pitch_motor_bracket.stl`: Pitch-axis GM3506 motor mounting bracket.
* `iris_payload_plate.stl`: Platform for smartphone/camera payload.

## **💻 How to Reproduce (Build Instructions)**
To ensure maximum reproducibility, follow these steps to build the project on a Raspberry Pi 5 running Linux:

```bash
# 1. Clone the repository
git clone [https://github.com/Realtime-Gimbal-Team/Realtime-CineFollow-Gimbal.git](https://github.com/Realtime-Gimbal-Team/Realtime-CineFollow-Gimbal.git)
cd Realtime-CineFollow-Gimbal

# 2. Create build directory
mkdir build && cd build

# 3. Compile the project
cmake ..
make -j4

# 4. Execute the Real-time Gimbal Node
./Realtime_CineFollow_Gimbal
```

## **💻 Software Architecture**

The software architecture is engineered following **SOLID principles** and the **Sense-Think-Act** paradigm. To meet strict real-time deadlines, the system is horizontally decoupled into independent threads, ensuring that heavy computer vision workloads never block critical motor control loops. 

All hardware communication and shared states are encapsulated in thread-safe classes with private data members, ensuring high reliability and ease of maintenance.

### 📂 Directory Structure & Module Decoupling

```text
📦 Realtime-CineFollow-Gimbal
 ┣ 📂 pico/Pico-firmware/       # Execution Layer: Microcontroller firmware
 ┃ ┣ 📂 SimpleFOC_Core/         # Hardware-level FOC motor driving algorithms
 ┣ ┣ 📂 include/                # Header files
 ┃ ┗ 📂 src/                    # High-frequency PID loop execution
 ┗ 📂 rpi_app/                  # High-Level Logic: Edge AI & Control
   ┣ 📂 3rdparty/ncnn/          # Optimized neural network inference engine
   ┣ 📂 include/                # Header files grouped by SOLID domains
   ┃ ┣ 📂 comm/                 # UartDriver: Serial protocol encapsulation
   ┃ ┣ 📂 control/              # Ramp generators and deadzone logic
   ┃ ┣ 📂 utils/                # SharedState: Thread-safe data brokers
   ┃ ┗ 📂 vision/               # VisionNode: Event-driven camera interface
   ┣ 📂 models/                 # Pre-trained YOLOv8-pose weights
   ┗ 📂 src/                    # Core C++ implementations (main.cpp, libcam interfaces)
```
🧱 Core Subsystems

**Sense**: Event-Driven Vision (vision/VisionNode)
Operating strictly on callbacks, the vision node retrieves zero-copy frame buffers from libcamera. It processes YOLOv8-pose inferences via NCNN and extracts pure pixel errors (target vs. anchor) without blocking the main CPU execution flow.

**Think**: Thread-Safe State Management (utils/SharedState)
To safely bridge the asynchronous vision pipeline and the deterministic control loop, the SharedState class acts as an exclusive data broker. It utilizes std::mutex and private properties with strict getter/setter interfaces, guaranteeing absolute thread safety and eliminating data races.

**Act**: Deterministic Dispatch (comm/UartDriver)
The control thread computes Symmetric Soft Ramps and dispatches continuous velocity commands. The UartDriver securely encapsulates these velocity primitives into structural packets and transmits them to the Pico-firmware.

**Execution**: Embedded FOC Controller (pico/Pico-firmware)
A dedicated Raspberry Pi Pico handles the lowest-level hardware interrupts. Running the SimpleFOC core, it translates abstract velocity commands into precise phase voltages, completely isolating the Raspberry Pi 5 from microsecond-level timing constraints.

## **📝3rd Party Components**

This project leverages several industry-standard open-source libraries to ensure high performance and reliability. We extend our gratitude to the maintainers of the following projects:

* **[NCNN](https://github.com/Tencent/ncnn)**: Tencent's high-performance neural network inference framework, highly optimized for mobile and edge platforms. 
    * *Usage in IRIS:* Powers the real-time inference of our YOLOv8-pose model on the Raspberry Pi 5 CPU, delivering ultra-low latency human tracking without needing a dedicated GPU.
* **[OpenCV 4.x](https://opencv.org/)**: The Open Source Computer Vision Library. 
    * *Usage in IRIS:* Utilized for zero-copy frame buffer handling, image format conversions (BGR2RGB), and core matrix manipulations before feeding data into the neural network.
* **[SimpleFOC](https://simplefoc.com/)**: An open-source Field Oriented Control (FOC) library for brushless motors. 
    * *Usage in IRIS:* Deployed on the auxiliary Raspberry Pi Pico to drive the GM3506 gimbal motors. It handles the high-frequency PID execution and translates velocity commands into precise phase voltages.
* **[libcamera](https://libcamera.org/)**: The standard Linux camera stack. 
    * *Usage in IRIS:* Interfaced directly via C++ to communicate with the Raspberry Pi Camera Module 3, bypassing legacy overhead to ensure raw, minimal-latency frame acquisition.

 
## **🧠 Reference & Tutorials**

The development of IRIS was grounded in rigorous academic research and adherence to professional embedded standards. Our work is built upon the following core theoretical and technical foundations:

### 🔬 Primary Technical Frameworks
* **[libcamera2opencv](https://github.com/berndporr/libcamera2opencv):** Developed by Prof. Bernd Porr. This repository served as the foundational driver for our vision pipeline, providing the critical C++ interface between `libcamera` and OpenCV mat structures on the Raspberry Pi 5.


### 👁️ Computer Vision & Edge AI
* **[YOLOv8 & Ultralytics](https://github.com/ultralytics/ultralytics):** The state-of-the-art object detection framework. We referenced the original architecture to optimize the model's depth-to-latency ratio for real-time edge inference.
* **[NCNN Optimization Guide](https://github.com/Tencent/ncnn/wiki/low-level-tuning):** Used for low-level tuning of ARM NEON instructions to accelerate convolution kernels on the Raspberry Pi 5's Cortex-A76 cores.
* **[Research Paper: "YOLOv8: Real-Time Object Detection and Beyond"](https://arxiv.org/abs/2301.05519):** Studied for understanding the loss functions and spatial attention mechanisms that guided our choice of detection anchors.

### 🕹️ Control Theory & Embedded Systems
* **[SimpleFOC Documentation](https://docs.simplefoc.com/):** The definitive guide for Field Oriented Control implementation. We utilized this for tuning the phase current loops of our GM3506 brushless motors.
* **[PID Control Theory - Ziegler-Nichols Method](https://en.wikipedia.org/wiki/Ziegler%E2%80%93Nichols_method):** Referenced for the systematic tuning of our dual-axis velocity and position loops to achieve cinematic stability.
* **[Symmetric Linear Ramp Generation](https://en.wikipedia.org/wiki/Linear_ramp_function):** Theoretical basis for our custom motion smoothing algorithms, ensuring non-jerky transitions during rapid target acquisition.

### ⚡ Real-Time Programming Standards
* **[POSIX Threads (pthreads) Documentation](https://man7.org/linux/man-pages/man7/pthreads.7.html):** Used as the standard for our multithreaded architecture to ensure portable and predictable thread behavior under Linux.
* **[Real-Time Linux (PREEMPT_RT) Concepts](https://rt.wiki.kernel.org/):** Studied to minimize interrupt latency and ensure our 100Hz control loop maintains high temporal determinism.

## **📊 Test & Assessment**

To ensure the IRIS gimbal meets strict industrial reliability and real-time execution standards, we conducted comprehensive quantitative testing on the physical prototype.

### 1. Real-Time Latency Profiling (Decoupled Architecture Validation)
In an event-driven architecture, it is critical to validate that heavy computational loads do not bottleneck motor execution. We implemented `std::chrono` high-resolution profilers to measure the true pipeline delay.

* **Vision Node Execution (Sense):** The NCNN YOLOv8-pose inference on the Raspberry Pi 5 CPU averages **85.0ms ~ 86.9ms** per frame (~11.5 FPS). This represents a highly authentic edge-AI workload limit without a dedicated TPU.
* **Control Loop Determinism (Think & Act):** Despite the ~86ms vision latency, our independent control thread computes the Symmetric Soft Ramp and UART dispatch in just **0.003ms**. This leaves a massive 9.997ms safety margin, ensuring the loop strictly adheres to its **100Hz (10ms) deadline**.
* **Architectural Triumph:** This data perfectly validates our multithreaded design. Because the 100Hz control loop is fully decoupled from the 11.5Hz vision pipeline, the gimbal utilizes our continuous deadzone compensator to interpolate movement, ensuring smooth, cinematic motor rotation rather than stuttering at the camera's framerate.

### 2. Failsafe Memory & Stability (Valgrind Profiling)
Given that IRIS is designed for continuous cinematography, memory leaks are unacceptable. We profiled the executable over a continuous run using **Valgrind** (`valgrind --leak-check=full`).

* **Result & Analysis:** The profiling reported a massive 216 MB of dynamic heap allocation during runtime. Crucially, our application-level C++ code achieved a **100% memory deallocation rate**. 
* The tool flagged a negligible `1 byte` definite loss and minor `possibly lost` blocks. Deep stack trace analysis confirms these are entirely isolated to third-party system shared libraries (`libtensorflow-lite.so` and `liblttng-ust.so`) during dynamic loading (`dl-init.c`), which are standard one-time initialization footprints on Linux and pose no risk of continuous leaking over time.
* **Conclusion:** Our strict adherence to modern C++ RAII principles successfully eliminated all application-level memory leaks, ensuring the gimbal can operate safely for extended filming sessions.

*(Note: Raw terminal profiling logs and structural assets are available in the `/assets/` directory).*
## **📢 Future Improvements**

While the current IRIS prototype successfully demonstrates real-time, event-driven cinematic tracking, several architectural and hardware upgrades are outlined for the next iteration to achieve true industrial-grade performance:

* **Edge-AI Vision Acceleration (Sub-30ms Latency):** To push the vision pipeline from ~11.5 FPS to over 30 FPS, we plan to implement NCNN's multi-threading capabilities and enable ARM NEON FP16 (half-precision) arithmetic. Additionally, downsizing the YOLOv8 input tensor from 640x640 to 416x416 will exponentially increase inference throughput with negligible loss in spatial tracking accuracy.
* **Payload Expansion & Infinite Yaw (Slip Rings):** The current GM3506 motors are optimized for smartphone payloads. Future mechanical iterations will integrate higher-torque motors (e.g., GM4108) and electrical slip rings, enabling infinite 360-degree continuous Yaw rotation for mirrorless and lightweight cinema cameras.
* **Deep Re-Identification (ReID) Integration:** Upgrading the current Greedy Distance Matching algorithm to a lightweight ReID neural network (such as OSNet). This will allow IRIS to memorize the specific visual features (clothing, colors) of a target, ensuring an absolute lock-on even in highly crowded environments with crossing subjects.
* **Autonomous Z-Axis (Auto-Zoom):** Integrating a third control axis via a follow-focus/zoom motor. By dynamically analyzing the bounding box depth scale in real-time, the system will autonomously adjust the lens focal length to maintain a constant subject framing proportion as they move across the z-axis.

---
<p align="left">
  <small>
    <b>🤖 AI Usage Disclosure:</b> 
Generative AI was utilized as a collaborative tool during development to assist in repository structural planning, technical documentation refinement (README/Manual), and C++ logic optimization. All core system architectures, hardware integrations, and real-time performance validations were manually implemented and verified for academic integrity.
  </small>
</p>
