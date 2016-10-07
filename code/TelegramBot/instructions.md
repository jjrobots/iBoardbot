Telegram for iBoardbot!
===================

Manage your iBoardbot from [Telegram](https://telegram.org/) with simple commands. Fun for all the family!

----------


Instructions
-------------

> **Create [Telegram Bot](https://core.telegram.org/bots):**

> 1. Open Telegram
> 2. Search [@BotFather](https://telegram.me/BotFather)
> 3. Start a conversation and write "/newbot"
> 4. Follow the steps to assign the name of the bot
> 5. Saves the token in a safe place
> 6. Talking to the BotFather you can change the settings anytime
> 
> If you want to see your bot commands, you must configure it in the BotFather:
>> 7. Open @BotFather
>> 8. Write "/setcommands" and copy:
	start - Start
	help - Gives you information about the available commands
    text - Send Message to iBoardbot
    textclear - Clear board and Send Message to iBoardbot
    clear - Clear iBoardbot
	
	
Installation
-------------------
In Linux:
> sudo apt-get install python-pip python-dev build-essential 
> sudo pip install --upgrade pip 
> sudo pip install --upgrade -r requirements.txt

Create Daemon
-------------------
Run the program always in your system
>sudo apt-get install -y supervisor
>sudo service supervisor start
>cd /etc/supervisor/conf.d/
>sudo nano iboardbot-tg.conf
>Copy and save:
>>[program:iboardbot-tg]
	command=/{{YOUR DIRECTORY}}/TelegramBot.py
	autostart=true
	autorestart=true
	startretries=3
	stderr_logfile=/var/log/iboardbot-tg.err.log
	stdout_logfile=/var/log/iboardbot-tg.out.log
	user=www-data

>supervisorctl reread
supervisorctl update
If your process is not running, try explicitly telling Supervisord to start process via "supervisorctl start iboardbot-tg"

More info about Supervisor: [Tutorial](https://stackedit.io/)

### Thanks!

