
# Muika Bot Installation on Linux

First, install the TgBot library by Oleg Morozenkov:

    sudo apt-get install -y g++ make binutils cmake libboost-system-dev libssl-dev zlib1g-dev libcurl4-openssl-dev;
    cd /tmp;
    git clone https://github.com/reo7sp/tgbot-cpp.git;
    cd tgbot-cpp;
    cmake .;
    make -j$(nproc);
    sudo make install;

After that, clone the Muika Bot repository:

    cd /path/to/your_dir;
    git clone https://github.com/ammarfaizi2/muika_bot.git;
    cd muika_bot;
    make -j$(nproc);
    export MUIKA_BOT_TOKEN="your telegram bot token";
    ./muika.bin;

I welcome patches and pull requests. Submit it on GitHub or send it to
my email. My email is:

  Ammar Faizi <ammarfaizi2@gnuweeb.org>

Please also CC:

  GNU/Weeb Mailing List <gwml@vger.gnuweeb.org>

if you submit it via email.

Join our Telegram group: https://t.me/GNUWeeb

-- 
Ammar Faizi
