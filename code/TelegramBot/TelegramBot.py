import telebot
import urllib3


TOKEN_BOT = "xxxxxxxxx" #Telegram Token Bot
APPID_IBOARDBOT = "xxxxxxxxx" #iBoardbot APPID


commands = {  # command description used in the "help" command
              'start': 'Get used to the bot',
              'help': 'Gives you information about the available commands',
              'text' : 'Send Message to iBoardbot',
              'textclear' : 'Clear board and send message to iBoardbot',
              'clear': 'Clear iBoardbot'
}

knownUsers = []  # todo: save these in a file,
userStep = {}  # so they won't reset every time the bot restarts

def iboardbot_send(mode, message):
        
        
        http = urllib3.PoolManager()
        
        if( mode == 1): #mode send text
            mode = "text"
        elif (mode ==2): #mode clear board and send text
            mode = "message"
        
        urltext = "http://ibbapp.jjrobots.com/api/v1/"+mode+".php?APPID="+APPID_IBOARDBOT+"&TEXT="
        url = urltext+message
        r = http.request('GET', url)
        

def iboardbot_clear(): #clear board
        
        http = urllib3.PoolManager()
        
        url = "http://ibbapp.jjrobots.com/api/v1/clear.php?APPID="+APPID_IBOARDBOT
        r = http.request('GET', url)


# only used for console output now
#def listener(messages):
   # for m in messages:
       # if m.content_type == 'text':
            # print the sent message to the console
            #print str(m.chat.first_name).encode('utf-8') + " [" + str(m.chat.id).encode('utf-8') + "]: " + m.text
            

bot = telebot.TeleBot(TOKEN_BOT)


# handle the "/start" command
@bot.message_handler(commands=['start'])
def command_start(m):
    cid = m.chat.id
    if cid not in knownUsers:  # if user hasn't used the "/start" command yet:
        knownUsers.append(cid)  # save user id, so you could brodcast messages to all users of this bot later
        userStep[cid] = 0  # save user id and his current "command level", so he can use the "/getImage" command
        bot.send_message(cid, "Hello, stranger, let me scan you...")
        msg = "Scanning complete, I know you now. Chatid: " + str(cid)
        bot.send_message(cid, msg)
        command_help(m)  # show the new user the help page
    else:
        bot.send_message(cid, "I already know you, no need for me to scan you again!")


# help page
@bot.message_handler(commands=['help'])
def command_help(m):
    
    cid = m.chat.id
    help_text = "The following commands are available: \n"
    for key in commands:  # generate help text out of the commands dictionary defined at the top
        help_text += "/" + key + ": "
        help_text += commands[key] + "\n"
    bot.send_message(cid, help_text)  # send the generated help page


#Send Message to iBoardbot
@bot.message_handler(commands=['text'])
def command_text(m):
    
    space = "%20"
    cid = m.chat.id
    text = m.text.split()
    
    num_text = len(text)
    
    message = space.join(text)
    
    if num_text > 2:
        message = space.join(text[1:])
    else:
        message = text[1]

    iboardbot_send(1,message)
    bot.send_message(cid, "Message sent correctly")


#Clear board and Send Message to iBoardbot   
@bot.message_handler(commands=['textclear'])
def command_textclear(m):
    
    space = "%20"
    cid = m.chat.id
    text = m.text.split()
    
    num_text = len(text)
    
    message = space.join(text)
    
    if num_text > 2:
        message = space.join(text[1:])
    else:
        message = text[1]

    iboardbot_send(2,message)
    bot.send_message(cid, "Message sent correctly")
    
    
#Clear iBoardbot
@bot.message_handler(commands=['clear'])
def command_clear(m):
    cid = m.chat.id
    iboardbot_clear()
    bot.send_message(cid, "Command sent correctly")
    


bot.polling(none_stop=True)
