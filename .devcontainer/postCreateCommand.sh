git submodule update --init
sudo apt update && sudo DEBIAN_FRONTEND=noninteractive apt upgrade -y
cd ..
wget https://raw.githubusercontent.com/raspberrypi/pico-setup/master/pico_setup.sh
chmod +x pico_setup.sh
./pico_setup.sh
echo "export PICO_SDK_PATH=/workspaces/pico/pico-sdk" > ~/.bashrc
source ~/.bashrc
echo -e "Finished.\n\n"