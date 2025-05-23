<?php
class Curl{

	var $debug = false;
	var $useGodaddyProxy = false;
	var $godaddyProxy = "http://64.202.165.130:3128";
	
	function __construct(){
		global $_Common;
		if(!function_exists('curl_version')){
			exit("<b>ERROR: Cannot Continue. Curl is not available through PHP</b>");
		}
	}
	
	// --------------------------------------------------------------
	function sendRequest($URL,$request){
		
		global $_Common;
		
		if(!$request){
			return false;
		}

		$result = NULL;

        if($this->debug){
            $_Common->debugPrint(curl_version(),"Curl Version:");
        }

        $ch = curl_init();
        if($this->debug){
	        curl_setopt($ch, CURLOPT_VERBOSE, 1);
	    }
	    if($this->useGodaddyProxy){
		    curl_setopt($ch, CURLOPT_PROXY, $this->godaddyProxy);
		}
        curl_setopt($ch, CURLOPT_URL, $URL);
        curl_setopt($ch, CURLOPT_HEADER, 0);
//marcello     curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($ch, CURLOPT_POST, 1);
        curl_setopt($ch, CURLOPT_POSTFIELDS, $request);
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);
		curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 0);
		//curl_setopt($ch, CURLOPT_SSLVERSION, 2);

        $result = curl_exec($ch) or die("There has been a cURL error connecting to $URL.");
        $info = curl_getinfo($ch);

        curl_close($ch);	

		if($this->debug){
			$_Common->debugPrint($result,"curl result");
			$_Common->debugPrint($info,"curl info");
		}
		
		return $result;
	}
}

?>