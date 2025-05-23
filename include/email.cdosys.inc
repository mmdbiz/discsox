<?PHP
		/*
	    class CdoConfiguration {
        static  cdoAutoPromoteBodyParts = "http://schemas.microsoft.com/cdo/configuration/autopromotebodyparts";
        const  cdoFlushBuffersOnWrite = "http://schemas.microsoft.com/cdo/configuration/flushbuffersonwrite";
        const  cdoHTTPCookies = "http://schemas.microsoft.com/cdo/configuration/httpcookies";
        const  cdoLanguageCode = "http://schemas.microsoft.com/cdo/configuration/languagecode";
        const  cdoNNTPAccountName = "http://schemas.microsoft.com/cdo/configuration/nntpaccountname";
        const  cdoNNTPAuthenticate = "http://schemas.microsoft.com/cdo/configuration/nntpauthenticate";
        const  cdoNNTPConnectionTimeout = "http://schemas.microsoft.com/cdo/configuration/nntpconnectiontimeout";
        const  cdoNNTPServer = "http://schemas.microsoft.com/cdo/configuration/nntpserver";
        const  cdoNNTPServerPickupDirectory = "http://schemas.microsoft.com/cdo/configuration/nntpserverpickupdirectory";
        const  cdoNNTPServerPort = "http://schemas.microsoft.com/cdo/configuration/nntpserverport";
        const  cdoNNTPUseSSL = "http://schemas.microsoft.com/cdo/configuration/nntpusessl";
        const  cdoPostEmailAddress = "http://schemas.microsoft.com/cdo/configuration/postemailaddress";
        const  cdoPostPassword = "http://schemas.microsoft.com/cdo/configuration/postpassword";
        const  cdoPostUserName = "http://schemas.microsoft.com/cdo/configuration/postusername";
        const  cdoPostUserReplyEmailAddress = "http://schemas.microsoft.com/cdo/configuration/postuserreplyemailaddress";
        const  cdoPostUsingMethod = "http://schemas.microsoft.com/cdo/configuration/postusing";
        const  cdoSaveSentItems = "http://schemas.microsoft.com/cdo/configuration/savesentitems";
        const  cdoSendEmailAddress = "http://schemas.microsoft.com/cdo/configuration/sendemailaddress";
        const  cdoSendPassword = "http://schemas.microsoft.com/cdo/configuration/sendpassword";
        const  cdoSendUserName = "http://schemas.microsoft.com/cdo/configuration/sendusername";
        const  cdoSendUserReplyEmailAddress = "http://schemas.microsoft.com/cdo/configuration/senduserreplyemailaddress";
        const  cdoSendUsingMethod = "http://schemas.microsoft.com/cdo/configuration/sendusing";
        const  cdoSMTPAccountName = "http://schemas.microsoft.com/cdo/configuration/smtpaccountname";
        const  cdoSMTPAuthenticate = "http://schemas.microsoft.com/cdo/configuration/smtpauthenticate";
        const  cdoSMTPConnectionTimeout = "http://schemas.microsoft.com/cdo/configuration/smtpconnectiontimeout";
        const  cdoSMTPServer = "http://schemas.microsoft.com/cdo/configuration/smtpserver";
        const  cdoSMTPServerPickupDirectory = "http://schemas.microsoft.com/cdo/configuration/smtpserverpickupdirectory";
        const  cdoSMTPServerPort = "http://schemas.microsoft.com/cdo/configuration/smtpserverport";
        const  cdoSMTPUseSSL = "http://schemas.microsoft.com/cdo/configuration/smtpusessl";
        const  cdoURLGetLatestVersion = "http://schemas.microsoft.com/cdo/configuration/urlgetlatestversion";
        const  cdoURLProxyBypass = "http://schemas.microsoft.com/cdo/configuration/urlproxybypass";
        const  cdoURLProxyServer = "http://schemas.microsoft.com/cdo/configuration/urlproxyserver";
        const  cdoUseMessageResponseText = "http://schemas.microsoft.com/cdo/configuration/usemessageresponsetext";
    };
    */

//-------------------------------------------------------------------
//$cc is expected to be a comma sperated list of addresses
function send_cdosys_mail($emailFromAddress,$emailFromName,$emailToAddress,$emailToName,
						  $subject,$bodyText,$bodyHTML,$cc=null,$bodyURL=null,$attachments = array()){
	global $_CF;
	
	$debug = false;
	

	// Use the PHP COM Object to access the CDO.Message object
	$Mail = new COM("CDO.Message");
	$mailConfig = new COM("CDO.Configuration");

	if(!$Mail){
		exit("Cannot connect to cdosys");
	}
	if(!$mailConfig){
		exit("Cannot configure cdosys");
	}

	if($debug){
		print "<pre>CDO From: $emailFromName < $emailFromAddress >\n";
		print "CDO To: $emailToName < $emailToAddress >\n";
		print "CDO Subject: $subject\n\n";
	}

	if(!defined('cdoPostUsingPickup')){
		//will only be defined already if auto reg of typelib is set
		//so on test works for all.
		define('cdoPostUsingPickup',  1);
		define('cdoSendUsingPort', 2);
	}
	
	
	$mailConfig->Load(-1);
	
	//Setup outgoing server info
	
	//setup server name and port number
	//if there is a colon in the server name then assume whats after is the port number
	$port=null;
	if(strstr($_CF['email']['mail_server_host'],':')!==false){
		list($server,$port) = explode(':',$_CF['email']['mail_server_host']);
	}else{
		$server = $_CF['email']['mail_server_host'];
	}
    $mailConfig->Fields['http://schemas.microsoft.com/cdo/configuration/smtpserver']->Value = $server;

    //use default port if one is not specified
    if(empty($port)){
		$port = 25;	
	}else{
		$port = intval($port);
	}
	$mailConfig->Fields['http://schemas.microsoft.com/cdo/configuration/smtpserverport']->Value = $port;
	$mailConfig->Fields['http://schemas.microsoft.com/cdo/configuration/sendusing']->Value = cdoSendUsingPort;

	//set 25 second timeout as many php servers time out at 30 on a request
    $mailConfig->Fields['http://schemas.microsoft.com/cdo/configuration/smtpconnectiontimeout']->Value = 25 ;
    
	//setup user name and password for the server if they are specifed.
    $user = $_CF['email']['mail_server_username'];
    $pass = $_CF['email']['mail_server_password'];
    if(!empty($user) || !empty($pass)){
		//use basic user/pass authentication
		$mailConfig->Fields['http://schemas.microsoft.com/cdo/configuration/smtpauthenticate']->Value = 1;  
		$mailConfig->Fields['http://schemas.microsoft.com/cdo/configuration/sendusername']->Value = $user;
		$mailConfig->Fields['http://schemas.microsoft.com/cdo/configuration/sendpassword']->Value = $pass;
	}else{
		//use anonymous so no authentication
		$mailConfig->Fields['http://schemas.microsoft.com/cdo/configuration/smtpauthenticate']->Value = 0;  
	}
	
	$mailConfig->Fields->Update();

	//now setup the mail object starting with the config we just setup    
	$Mail->Configuration = $mailConfig;
    
    //setup to
    $tempAddress = $emailToAddress;
    $tempName="";
    if(!empty($emailToName)){
		$tempName = $emailToName;
		$tempAddress = "<$tempAddress>";
	}
    $Mail->To = "$tempName $tempAddress";
    
    //setup from
    $tempAddress = $emailFromAddress;
    $tempName="";
    if(!empty($emailFromName)){
		$tempName = $emailFromName;
		$tempAddress = "<$tempAddress>";
	}
    $Mail->From = "$tempName $tempAddress";
    
    $Mail->Subject = $subject;
    
    if(!empty($cc)){
		$Mail->CC = $cc;
	}
	
	if(!empty($bodyURL)){
		$Mail->CreateMHTMLBody($bodyURL);
	}elseif(!empty($bodyHTML)){
        if(stristr($bodyHTML,"<text>")){
	       $bodyNew = preg_replace("/<text>/i","<text>",$bodyNew);
    		//now set the body text
            $aHTML = explode("<text>",$bodyNew);        
            $Mail->HTMLBody = $aHTML[0];
        }
        else{
            $Mail->HTMLBody = $bodyHTML;
        }
	}

	if(!empty($bodyText)){
		//reset the text body if we were given one.
		$Mail->TextBody = $bodyText;
	}

	// attachments must have full filepaths
	if(count($attachments) > 0){
		foreach($attachments as $i=>$attachment){
			$Mail->AddAttachment($attachment);
		}
	}

    $result = $Mail->Send();

	$mailConfig->Release();
	$mailConfig= null;
	
	$Mail->Release();
	$Mail = null;

	if($debug){
        print "CDO Mail Sent...$result\n</pre>";
    }
    
	return true;	
    
}

?>