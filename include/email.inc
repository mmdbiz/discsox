<?php

class Email{

	var $debug = false;
	var $sent = false;

	// --------------------------------------------------------------
	function __construct(){
		global $_CF, $_DB;
	
		// add this new CF param if missing
		if(!isset($_CF['email']['use_customer_email_as_from_address'])){
			$row = $_DB->getRecord("SELECT MAX(sequence) as seq FROM config WHERE section = 'email'");
			$seq = $row['seq'] + 1; 
			$sql = "INSERT INTO config (`section`,`sequence`,`key`,`value`) VALUES('email','$seq','use_customer_email_as_from_address','true')";
			$_DB->execute($sql);
			$_CF['email']['use_customer_email_as_from_address'] = 'true';
			
			$count = $_DB->getCount("help","WHERE `key` = 'use_customer_email_as_from_address'");
			if($count == 0){
				$sql = "INSERT INTO help (`section`,`key`,`key_help`) 
						VALUES('email','use_customer_email_as_from_address',
							   'Allows you to set the from address for the store email to the customer email address.<br /><br /><b>NOTE:</b> This may not work on some email servers.')";
				$_DB->execute($sql);
			}
		}
	}

	// --------------------------------------------------------------
    function send($emailfrom, $emailto, $subject, $html, $text, $url = null){

        global $_CF;
		global $_Common;
		
		//Disable stric error deprecated and notices
		error_reporting(E_ALL & ~E_STRICT & ~E_NOTICE & ~E_DEPRECATED);

        if(!$_CF['email']['send_email']){
            return;
        }

		if($this->debug){
			print "<pre>Email from: $emailfrom\n";
			print "Email to: $emailto\n";
			print "Subject: $subject\n";
			//print "HTML: $html\n";
			//print "Text: $text</pre>\n";
			print "</pre>\n";
		}

		$storeName = $_CF['basics']['store_name'];
		
		//clean customer name from special characters that prevent sending
		$specChars = array("@", ".");
		$cleanCustName = str_replace($specChars, " ", $emailto);		
		$customerName = $cleanCustName;
		
		if(!empty($_SESSION['billaddress_firstname'])){
			$customerName = $_SESSION['billaddress_firstname'] . " " . $_SESSION['billaddress_lastname'];
		}

		$custEmail = NULL;
		if(!empty($_SESSION['billaddress_email'])){
			$custEmail = $_SESSION['billaddress_email'];
		}

        $storeEmail = trim($_CF['email']['store_email_address']);

		//email is assumed to come from the store to customer
		$emailfrom = $storeEmail;
		$emailFromName = $storeName;
		$emailToName = $customerName;
		
		//if target is store then set cc as needed and adjust ToName
        $ccAddrs = array();
        if(trim($emailto) == $storeEmail){
			//mail is heading to store address from the store
            //$ccAddrs = split(",",$_CF['email']['store_cc_email_addresses']);
            $ccAddrs = explode(",",$_CF['email']['store_cc_email_addresses']);
            $emailToName = $storeName;
        }

		// If Windows server, use cdosys
        if($_CF['email']['use_cdosys'] != 'false' && $_CF['email']['use_cdosys'] !== false){
			
            include_once('email.cdosys.inc');

            $cc = null;
            if(count($ccAddrs) > 0){
                $cc = join(",",$ccAddrs);
            }
			// With cdosys we can use the customer email as the from email
			// because we are logging into the mail server using authentication.
            if(trim($emailto) == $storeEmail){
				if(isset($_CF['email']['use_customer_email_as_from_address']) && $_CF['email']['use_customer_email_as_from_address']){
					$emailFromName = $customerName;
					$emailfrom = $custEmail;
				}
	        }
            $result = send_cdosys_mail($emailfrom,$emailFromName,$emailto,$emailToName,$subject,$text,$html,$cc,$url);
            if($result){
				$this->sent = true;	
			}	
        }
        else{
			// use pear mail including mime
			 require_once "Mail.php"; 
			 require_once 'Mail/mime.php';
			 
			 $crlf = "\n";
			 $host = "ssl://mmdesign.com";
			 $port = "465";
			 			
			//check for cc
			$cc = "";
            if(count($ccAddrs) > 0){
                $cc = join(",",$ccAddrs);
            }
            
			//check for "to email" to be store address
			if(trim($emailto) == $storeEmail){
				//check to use customer email as from address
				if(isset($_CF['email']['use_customer_email_as_from_address']) && $_CF['email']['use_customer_email_as_from_address']){
					//if(!empty($_REQUEST['billaddress_email'])){ //original
					if(!empty($_SESSION['billaddress_email'])){ //Fixed marcello 8/24/21
						//$custEmail = $_REQUEST['billaddress_email']; //original
						$custEmail = $_SESSION['billaddress_email']; //Fixed marcello 8/24/21
						$returnPath = $custEmail;
						$from = "$customerName <$custEmail>";
					}
				}
				else{
					$returnPath = $emailfrom;
					$from = "$storeName <$emailfrom>";
				}
	            $to = "$emailToName <$emailto>";
			}
			else{
				$returnPath = $emailfrom;
	            $from = "$storeName <$emailfrom>";
	            $to = "$customerName <$emailto>";
	        }
			
			
			if($this->debug){
				print "<pre> \n";
				print "Store email: $storeEmail\n";
				print "Customer name: $customerName\n";
				print "emailToName: $emailToName\n";
				print "Email from: $from\n";
				print "Email to (to variable): $to\n";
				print "Email to (emailto variable): $emailto\n";
				print "CC addresses: $cc\n";
				//print "HTML: $html\n";
				//print "Text: $text</pre>\n";
				print "</pre>\n";
			}

			
	        //email is good to go now rework the smtp parms if we need to.
	        //if there is a colon in the server name then assume whats after is the port number
			$port=null;
			$helo = null;
			if(strstr($_CF['email']['mail_server_host'],':')!==false){
				list($server,$port) = explode(':',$_CF['email']['mail_server_host']);
			}else{
				$server = $_CF['email']['mail_server_host'];
			}
	        
			if ($_CF['email']['connection_encryption'] == "none"){
				$host = $server;
			}else{
				$host = $_CF['email']['connection_encryption'] . "://" . $server;
			}
			 
	       	//setup user name and password for the server if they are specifed.
			$user = null;
			$pass = null;
			$auth = null;
			if(!empty($_CF['email']['mail_server_username']) || !empty($_CF['email']['mail_server_password'])){
				$user = $_CF['email']['mail_server_username'];
				$pass = $_CF['email']['mail_server_password'];
				$auth = true;
			}
			//create mail headers array
			 $headers = array (
			   'From' => $from,
			   'To' => $to,
			   'Subject' => $subject,
			   'Return-Path' => $returnPath);
			   
		   // Creating the Mime message
			$mime = new Mail_mime($crlf);
	  
			// Setting the body of the email
			  $mime->setTXTBody($text);
			  $mime->setHTMLBody($html);
		  
			  $body = $mime->get();
			  $headers = $mime->headers($headers);
			 	 		 
			 $smtp = Mail::factory('smtp',
			   array (
			     'host' => $host,
				 'port' => $port,
				 'auth' => true,
				 'username' => $user,
				 'password' => $pass));
				 
				 
			if($this->debug){
				print "<pre> \n";
				print "host: $host\n";
				print "port: $port\n";
				print "</pre>\n";
			}
	
			 $mail = $smtp->send($to, $headers, $body);
	        //$mail->setSMTPParams($server, $port, $helo, $auth, $user, $pass);
	        
//			if($_CF['email']['use_smtp_to_send_mail']){
//				$result = $mail->send(array($to), 'smtp');
//			}
//			else{
//				$result = $mail->send(array($to));
//			}
	        
	        // debug code
	        if($this->debug){
				if (PEAR::isError($mail)) {
				 echo("<p>" . $mail->getMessage() . "</p>");
				} else {
				 echo("<p>Message successfully sent!</p>");
				}
//				if(!$result) {
//					print_r($mail->errors);
//				}
//				else{
//					if($_CF['email']['use_smtp_to_send_mail']){
//						echo 'SMTP Mail sent!';
//					}
//					else{
//						print "PHP Mail sent!";
//					}
//				}
			}
			
			if (!(PEAR::isError($mail))) {
				$this->sent = true;	
			}			
//			if($result){
//				$this->sent = true;	
//			}			
        }
    }
}

?>
