# Invisible ESP8266 PID controlled Gaggia Classic Pro
This is another PID controller for the Gaggia Classic (Pro) espresso portafilter machine. I didn't want a chunky box hanging out of my otherwise clean coffee machine setup, so I made a PID controller using the ESP8266, MAX6675, and an SSR controlled by a polished web interface optimized for mobile devices.
All the parts can be comfortably accommodated within the Gaggia, so it looks just like a stock machine from the outside. It can also still be operated manually without consulting the web interface. The mod is always easily reversible without leaving any traces.

<p align="center">
  <img src="https://raw.githubusercontent.com/da-luggas/gaggia-pid-presto/main/screenshot.jpeg" width="350">
</p>

## Functionality
- [ ] Check the current temperature of the boiler
- [ ] Switch between brewing and steaming mode
- [ ] Set the temperature for both brewing and steaming
- [ ] Set PID parameters (Kp, Ki, Kd)
- [ ] Retain settings throughout reboots (using EEPROM)

## Part list
This is the bare minimum you will need for this project. I sourced most of the parts from AliExpress. My total costs were around just **$15**. However, you want to have some 3D-printed enclosure for the hardware, which I did not include in the parts and costs. You will also need tools (screwdrivers, adjustable wrench, soldering iron, solder, double-sided tape, etc.)

Please note that the products linked are not recommended items; they are simply the cheapest and most available when I got them.

- [ ] [Wemos D1 Mini development board (ESP8266)](https://www.aliexpress.com/item/1005004544650251.html?spm=a2g0o.order_list.order_list_main.23.7cef5c5ff2fnuL)
- [ ] [MAX6675 Thermocouple sensor board](https://de.aliexpress.com/item/1005004884357182.html?spm=a2g0o.order_list.order_list_main.5.7cef5c5ff2fnuL&gatewayAdapt=glo2deu)
- [ ] [Solid State Relay (40A)](https://www.aliexpress.com/item/1005004040211802.html?spm=a2g0o.order_list.order_list_main.17.7cef5c5ff2fnuL)
- [ ] [Generic Mini Power Supply](https://www.aliexpress.com/item/4001025950728.html?spm=a2g0o.order_list.order_list_main.29.7cef5c5ff2fnuL)
- [ ] [M4 threaded Type K thermocouple](https://de.aliexpress.com/item/4000273387109.html?spm=a2g0o.productlist.main.5.418057054nsvD1&algo_pvid=3ef062e5-78f9-4295-aea4-f42d54f9878f&aem_p4p_detail=202401120317222250661719526920001418867&algo_exp_id=3ef062e5-78f9-4295-aea4-f42d54f9878f-2&pdp_npi=4%40dis%21EUR%215.27%214.85%21%21%215.65%215.20%21%40211b61ae17050582420524861e9223%2110000001111691064%21sea%21SK%214368421280%21&curPageLogUid=cL0mWBZQ8kfh&utparam-url=scene%3Asearch%7Cquery_from%3A&search_p4p_id=202401120317222250661719526920001418867_3)
- [ ] [2x 6.3 mm Piggyback terminals](https://de.aliexpress.com/item/1005005570418532.html?spm=a2g0o.productlist.main.21.29093807fx9Lic&algo_pvid=b3ab4411-3dc2-4117-a1e1-c953dbc501c9&algo_exp_id=b3ab4411-3dc2-4117-a1e1-c953dbc501c9-10&pdp_npi=4%40dis%21EUR%212.29%211.63%21%21%212.46%211.75%21%40211b61ae17050584108007951e9223%2112000033598638848%21sea%21SK%214368421280%21&curPageLogUid=Zu86sn9TmSjs&utparam-url=scene%3Asearch%7Cquery_from%3A)
- [ ] [2x 6.3 mm male spade connectors](https://de.aliexpress.com/item/1005002765359666.html?spm=a2g0o.productlist.main.33.5e79eecc2FGiCX&algo_pvid=115b535c-c450-4063-9099-bfcf09a9d188&algo_exp_id=115b535c-c450-4063-9099-bfcf09a9d188-16&pdp_npi=4%40dis%21EUR%213.88%211.55%21%21%214.16%211.66%21%40211b61ae17050584698491357e9223%2112000022078614611%21sea%21SK%214368421280%21&curPageLogUid=hmekfISx024F&utparam-url=scene%3Asearch%7Cquery_from%3A)
- [ ] 1m 2.5 mm2 wire
- [ ] 1m 1.5 mm2 wire

## Installation instructions

### Hardware
I pretty much followed the steps described in this very well-made video: https://www.youtube.com/watch?v=gj9qLIDaF9g
You can just find the layout you want yourself and where you want to hide the parts inside the machine. You can also just route them outside to have easier access to the ESP.

### Software
#### Using the pre-built binaries
We are going to use esptool to flash the binary to your ESP8266

1. Check the GitHub release tab and download the latest `.bin` release.
2. Install Python on your machine (Necessary on [Windows](https://www.python.org/downloads/windows/), macOS and Linux usually come with Python installed by default)
3. Install esptool by running `pip install esptool` in your command line
4. Connect the ESP8266 to your computer and find the serial port

	**Linux**: run `ls /dev/ttyUSB*` in your terminal
   
	**macOS**: run `ls /dev/tty.*` in your terminal
   
	**Windows**: Device Manager > Ports (COM & LPT) > Right click on device > Properties > Port Settings
   
	Please note that on Windows, you might also have to install the [USB Drivers for the NodeMCU](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads)
6. Flash the binary:

	`esptool.py -chip esp8266 erase_flash`

	`exptool.py -chip esp8266 -port <PORT_FROM_STEP_4> write_flash -z 0x1000 /path/to/your/firmware.bin`

### First start
After you have set up everything and turned on your ESP8266 for the first time, you need to set up the WiFi connection. Search for the WiFi network called "Gaggia Classic" on your phone. It is an open network without password protection. A window will pop up once you connect to it, guiding you through connecting the ESP to your home network. Once the connection is successful, you can reach the control panel by navigating to your ESP8266's IP address (you can find it in your router).

## Planned features
- [ ] Progressive Web App (so that the website can be pinned to the home screen)
- [ ] Automatic shot timer using solenoid
- [ ] Adjust website colors to fit better to the topic of coffee

---

## Contributing to the Project

I welcome contributions from coffee enthusiasts and tech wizards alike! If you're interested in improving the project, here's how you can contribute:

### Getting Started

1. **Fork the Repository**: Start by forking the repository to your own GitHub account. This creates a personal copy for you to work on.

2. **Clone the Forked Repository**: Clone your forked repository to your local machine. This allows you to work on the project in your own development environment.

   ```bash
   git clone https://github.com/da-luggas/gaggia-pid-presto.git
   cd gaggia-pid-presto
   ```

### Making Changes

1. **Create a New Branch**: Before you start making changes, create a new branch. This helps in isolating your changes from the main project.

   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make Your Changes**: Implement your feature, fix a bug, or make other changes. Ensure your code adheres to the existing style of the project to maintain consistency.

3. **Commit Your Changes**: Once you're satisfied with your changes, commit them to your branch. Write clear, concise commit messages to explain your changes.

   ```bash
   git commit -am "Add a concise and clear description of your changes"
   ```

### Submitting Your Changes

1. **Push to GitHub**: Push your branch with the changes to your fork on GitHub.

   ```bash
   git push origin feature/your-feature-name
   ```

2. **Create a Pull Request**: Go to the original repository on GitHub. You'll see a prompt to create a pull request from your new branch. Fill in the details, explaining the purpose of your changes.

3. **Code Review**: Wait for the project maintainer to review your changes. Be open to feedback and be ready to make revisions if requested.

### Final Steps

- Once your pull request is approved and merged, your contributions will be part of the project! 
- Don't forget to pull the changes from the original repository to keep your fork up to date.

   ```bash
   git checkout main
   git pull upstream main
   git push origin main
   ```

### Thank You!

Your contributions play a vital role in the continuous development of this project. Whether it's a minor bug fix, a feature enhancement, or a documentation improvement, every contribution is valuable. Please have a look at the planned features section of this README for some inspirations what could be a good addition to the project.

Happy coding, and enjoy your enhanced coffee experience!
