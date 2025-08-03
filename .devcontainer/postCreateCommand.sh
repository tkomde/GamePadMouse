sudo apt update && sudo apt upgrade -y
wget https://raw.githubusercontent.com/raspberrypi/pico-setup/master/pico_setup.sh
chmod +x pico_setup.sh
./pico_setup.sh
echo "export PICO_SDK_PATH=/workspaces/GamePadMouse/pico/pico-sdk" > sh.txt