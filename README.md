# MET Industrial IoT: teaching tools

Interactive pages for the IfM MET Industrial IoT lecture, served with GitHub Pages.

- `cold-store/` — **Cold-store monitor builder.** Students pick a sensor, a radio link, what runs on the device and who acts at 3 am, and see kit cost, battery life, false alarms and time above 8 °C when the compressor fails at 02:00. Each design gets a four-digit code that the lecturer enters on the results wall.
- `cold-store/wall/` — **Results wall** for the lecturer’s screen. Designs are kept in that browser only (nothing is sent anywhere); use “Download results (CSV)” to keep a copy.

Live links (work even while GitHub Pages deploys are delayed):
- Builder: https://htmlpreview.github.io/?https://github.com/anandarupmukherjee/teaching_met_IoT/blob/main/cold-store/index.html
- Wall: https://htmlpreview.github.io/?https://github.com/anandarupmukherjee/teaching_met_IoT/blob/main/cold-store/wall/index.html

The pages are single self-contained HTML files with no build step and no server-side data collection. The figures in the model are typical values for teaching, not supplier quotes.
