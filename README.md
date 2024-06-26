
# Honours 2024: Robosub motor test setup

## Content

- [Intro](#Introduction)
- [Getting Started](#getting-started)
  - [Installation](#Installation)
- [To-Do](#to-do)
- [Sharepoint](#sharepoint)

### Introduction
Software for the motor test setup. Allows to measure and plot the power and force of the motor.

## Getting Started

### Installation 
1. Create a Github Account
2. Download [git.exe](https://git-scm.com/downloads) / [Github desktop](https://desktop.github.com/) 
3. Download VS-code & the [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)
4. Install [VS-code github extension](https://marketplace.visualstudio.com/items?itemName=GitHub.vscode-pull-request-github)
6. Clone this repo (git clone)
7. Open VS-code workspace
8. Make a branch of the main per new SW function

Error:  
Could not find one of the manifest files ‘package.json’

Solution:  
1. Remove C:\Users\USERNAME\\.platformio folder
2. Restart VS-code
3. PlatformIO reinstalls itself
4. Retry compilation

## To-Do
- [ ] test readVernier force sensor
- [ ] test calibrating
- [ ] validate calcPower output
- [X] write function LCD screen
- [X] test LCD screen NOTE: I2C_address
- [X] write function servo program
- [ ] motorTest() with timers
- [ ] test servo program 0 - 100 %
- [x] UART / serialplotter output
- [x] write analogRead calibration mode for pins A0 voltage & A1 Current

## Sharepoint
<a href="https://hogeschoolutrecht.sharepoint.com/sites/ext_Onderzoek_robosub/Gedeelde%20documenten/Forms/AllItems.aspx" target="_blank"><img src="images/RoboSub_logo.png" alt="logo RoboSub"  width="100"/> <br> RoboSub (EXT) </a>

[Microsoft Teams - Robosub (EXT)](https://teams.microsoft.com/l/team/19%3A15CCBrzAxFNYSKObK1CGoetBEwihwAm-_twrglIF83A1%40thread.tacv2/conversations?groupId=6f8a9cd1-c804-4121-ba97-c6f4a1464002&tenantId=98932909-9a5a-4d18-ace4-7236b5b5e11d) 

<a href="https://hogeschoolutrecht.sharepoint.com/:f:/s/int_Onderwijs_honours-2023/Eiajz2l_SetFsb1aE4gwhkwBwOuPde7210CmGLjKcM_mBA?e=UAcvJS" target="_blank">Hardware schematics </a>
