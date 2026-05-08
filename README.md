# STM32F4 Environmental Monitoring System

This project implements a real-time environmental monitoring system on the 
STM32F407G-DISC1 using FreeRTOS to periodically sample, process, and analyze 
data from AHT20, BH1750, and BMP180 sensors. To ensure data reliability, the 
system utilizes a sliding-window median filter alongside ring buffer management 
to smooth out sensor noise before calculating statistical metrics, while also 
leveraging SEGGER SystemView for precise task profiling and CPU optimization.

If you would like to explore the detailed source code architecture and API 
references, please visit the 
[Doxygen Documentation](https://suledemirdas.github.io/STM32F4-Environmental-Monitoring/).

You can access the full project documentation [here](https://github.com/SuleDemirdas/STM32F4-Environmental-Monitoring/blob/develop/Project%20Report.pdf).
