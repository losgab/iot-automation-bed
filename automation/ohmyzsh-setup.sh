curl https://raw.githubusercontent.com/losgab/iot-automation-bed/main/automation/.zshrc > .zshrc
exec zsh -l
sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"