# MET Industrial IoT: teaching tools

Interactive pages for the IfM MET Industrial IoT lecture, served with GitHub Pages.

- `cold-store/` — **Cold-store monitor builder.** Students pick a sensor, a radio link, what runs on the device and who acts at 3 am, and see kit cost, battery life, false alarms and time above 8 °C when the compressor fails at 02:00. Each design gets a four-digit code that the lecturer enters on the results wall.
- `m5-plane/` — **Flight twin.** An M5Stack publishes its IMU (accel + gyro, 100 Hz in batches of 10) over Wi‑Fi to the public MQTT broker `broker.hivemq.com`, topic `ifm-met/iiot/m5/<id>/imu`. The page subscribes over WebSockets, fuses the readings into an orientation (Madgwick filter) and computes vibration RMS and spectrum. Firmware: `m5-plane/firmware/m5_imu_stream/`. URL options: `?id=` device id, `?demo=1` simulated data, `?kiosk=1` hides the help.
- `cold-store/wall/` — **Results wall** for the lecturer’s screen. Designs are kept in that browser only (nothing is sent anywhere); use “Download results (CSV)” to keep a copy.

Live links: https://anandarupmukherjee.github.io/teaching_met_IoT/cold-store/ (builder) and /cold-store/wall/ (wall).

Backup links (work even while GitHub Pages deploys are delayed):
- Builder: https://htmlpreview.github.io/?https://github.com/anandarupmukherjee/teaching_met_IoT/blob/main/cold-store/index.html
- Wall: https://htmlpreview.github.io/?https://github.com/anandarupmukherjee/teaching_met_IoT/blob/main/cold-store/wall/index.html

The pages are single self-contained HTML files with no build step and no server-side data collection. The figures in the model are typical values for teaching, not supplier quotes.
