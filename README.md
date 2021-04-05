# EAuthenticator
ESurfing thirdparty desktop client.

Features:  
1.You can login automatically.  
2.There is no need to confirm the UAC.  
3.Startup params for server.  
  
How to use:  
1.Use startup params.    
    -username: Your account.  
    -password: Your account password.  
    -auto: Login automatically.   
Sample:  
    EAuthenticator.exe -username 1234 -password=1234  
Notice:
    It will login with incoming params first.If login success,the incoming account will save to an encrypted file.   
    Use "-auto" param only that login with last account automatically.  

2.In GUI,enter your account and password,then click the button,if you use this tool for the first time,it will require a NASIP.    
Notice:  
    About NASIP.You can get the information in the dialogue.    
   
FAQs:  
1:How can I login automatically?    
A:Add a Windows startup item with startup param "-auto".     
  
2:Why it always reminds me that some dlls is missing?  
A:Please install the VC++ Redistributable Package 2010-2019.  

Thirdparty API:  
jsoncpp->https://github.com/open-source-parsers/jsoncpp  
