<?php

class Initialize{

    public bool $debug = false;
    public bool $debug2 = false;
	public $cnx = null;
	public $includePath = null;
    
	function __construct($isReport = false,$isUtility = false){

		global $_isAdmin;
		global $_CF;
		global $_Common;
		global $_Config;
		global $_CR;
		global $_DB;
		global $ip_address;
		global $_Registry;
		global $_Session;
		global $_Template;
		global $website_url;
		global $secure_url;
		global $shipToBilling;

		ini_set('display_errors','on');
		//marcello disable deprecated and warning messages		
		//error_reporting(E_ALL);
		error_reporting(E_ALL);

		ini_set("short_open_tags", 'On');

        if($this->debug){
			echo "cdw: " . getcwd() . "\n <br>";
		}
		if($_isAdmin){
			if($isReport || $isUtility){
				chdir("../../");
			}
			else{
				chdir("../");
			}
		}
        if($this->debug){
			echo "cdw: " . getcwd() . "\n <br>";
		}

        if(!empty($_SERVER['REMOTE_ADDR'])){
            $ip_address = $_SERVER['REMOTE_ADDR'];
        }
		elseif(!empty($_SERVER['REMOTE_HOST'])){
			$ip_address = $_SERVER['REMOTE_HOST'];
		}

		// Set local include paths
		$this->setIncludePath();
		
		include_once("include/startup.inc.php");

		$_CR = "\n";
		if(getenv("windir") != ""){
			$_CR = "\r\n";
			if($this->debug){
				echo "we are in a Windows dir";
			}
		} 
		else {			
			if($this->debug){
				echo "we are in a Linux dir";
			}
		}

		// Add this define so we can read the config
		if(!defined('INIT.CART')){
			define('INIT.CART', true);
		}
		
		// Include general functions
		include_once("include/common.inc");
		$_Common = new Common();

		// Load Registry
		include_once("registry.inc");
		$_Registry = New Registry($this->includePath);
		
//		// save the DisplayDevice session variable - marcello
//		if (isset($_SESSION['DisplayDevice'])) {
//		  $DisplayDevice = $_SESSION['DisplayDevice'];
////echo "got here 1...";
//		}
////		in case the session variable is not set default to desktop
//		else {
//			 $DisplayDevice = 'desktop';
////echo "got here 2...";
//		}
//		session_write_close();

		
		// Load and start session
		//echo "got here too..."; // marcello
		$_Session = $_Registry->LoadClass("session");

		//start session
		$this->cnx = $_Session->Start(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME, SESSION_TIMEOUT);

		// Clean up user input in $_REQUEST
		$this->cleanupRequest();
		
		//restore the session variable - marcello
		//$_SESSION['DisplayDevice'] = $DisplayDevice;

		// load general database class
		$_DB = $_Registry->LoadClass("db");

		// load general database class
		$_Config = $_Registry->LoadClass("config");
		$_Config->read(NULL,true);


//$_SESSION['DisplayDevice'] = $DisplayDevice; //marcello

		// load template class
		$_Template = $_Registry->LoadClass("template");

		// Save last page
		if(empty($_SESSION['last_page'])){
			$_SESSION['last_page'] = $_CF['basics']['home_page_name'];
		}		
		
		//marcello		
		$shipToBilling = $_CF['basics']['ship_to_billing'];
		
		// Website/secure url
		$website_url = $_CF['basics']['web_site_url'];
		if(!empty($_CF['basics']['secure_url'])){
			$secure_url = $_CF['basics']['secure_url'];
		}
		else{
			$secure_url = $website_url;
		}

		if($_isAdmin){
			if($isReport){
				chdir("./cp/reports");
			}
			elseif($isUtility){
				chdir("./cp/utilities");
			}
			else{
				chdir("./cp");	
			}
		}

		if(!defined('ISMENU')){
			if(!$_isAdmin && isset($_CF['basics']['maintenance_mode']) && $_CF['basics']['maintenance_mode']){
				$_Common->printErrorMessage($_CF['messages']['maintenance_title'],$_CF['messages']['maintenance_message']);
			}
			elseif(!$_isAdmin && !isset($_CF['basics']['maintenance_mode'])){
				if(file_exists('cp/database/maintenance.mode.sql')){
					include_once('cp/database/maintenance.mode.sql');	
				}
			}
		}
	}
	// ----------------------------------------------------
	function cleanupRequest(){

		global $_isAdmin;
		global $_Common;

		if(isset($_REQUEST['page'])){
			unset($_REQUEST['page']);
		}

		if(isset($_COOKIE)){
			// remove cookies
			if(!$_isAdmin){
				foreach($_COOKIE as $ck=>$cv){
					unset($_REQUEST[$ck]);	
				}
			}
		}

		$vars = $_REQUEST;
		$_REQUEST = array();
        if($this->debug2){
			$_Common->debugPrint($vars,"REQUEST before cleanup");
				echo("got here too");
		}
		$_REQUEST = $this->cleanUpInput($vars);
		
	}
	
	//---------------------------------------------------------------
	function cleanUpInput($input){
		
		global $_Common;
		
		// this cleans up the keys and the values of the inputs
		
		$output = array();
		foreach($input as $key=>$value){

			// clean it up
			$key = trim(preg_replace("/<script/i","",$key));
			$key = trim(str_replace(">","",$key));
			$key = trim(str_replace(";","",$key));
			$key = trim(str_replace('--',"",$key));
			$key = trim(str_replace('exec',"",$key));
			$key = trim(preg_replace("/[\"\\\&\`\;\*\^\!\(\)\{\}\<\>\n\r\=]/","",$key));
			
			if(is_array($value)){
				$output[$key] = $this->cleanUpInput($value);
			}
			else{
				$output[$key] = $this->escape_string($value);
			}
		}
		return $output;
	}


	//---------------------------------------------------------------
	function escape_string($string){
		
		global $_isAdmin,$_Common;
	
		if(!$_isAdmin){
			$string = trim(preg_replace("/<script/i","",$string));
			$string = trim(str_replace(">","",$string));
			$string = trim(str_replace(";","",$string));
			$string = trim(str_replace('--',"",$string));
			$string = trim(str_replace('exec',"",$string));
			//$string = trim(preg_replace("/[\"\\\&\`\;\*\^\!\(\)\{\}\<\>\n\r\=]/","",$string));
			$string = trim(preg_replace("/[\"\\\&\`\;\*\^\!\(\)\{\}\<\>\=]/","",$string));
			$string = nl2br($string);
		}

        global $_DB; // Assumes $_DB is initialized and has a valid cnx property
        if (isset($_DB) && $_DB->cnx) {
            $string = mysqli_real_escape_string($_DB->cnx, $string);
        } else {
            // Fallback if $_DB or its connection isn't available
            // This might indicate an issue with initialization order or scope
            trigger_error('escape_string in Initialize class called without a valid global DB connection.', E_USER_WARNING);
            $string = addslashes($string);	
        }
		
		return $string;
	}


	// ----------------------------------------------------
    function setIncludePath(){

		global $_isAdmin;
		
		// Get the current include path and push our local
		// path in the front. This allows you to use a 
		// global set of includes as set in the php.ini
		
        $ps = PATH_SEPARATOR;
        $ds = DIRECTORY_SEPARATOR;
        $cwd = getcwd();
       
		$new_ini_value = ".";

		// Add paths we might use
		$new_ini_value .= $ps . $cwd . $ds . 'qs30';
		//$new_ini_value .= '/home/mmdesi5/php';
		$new_ini_value .= $ps . $cwd . $ds . '..' . $ds . 'qs30';
		$new_ini_value .= $ps . $cwd . $ds . '..' . $ds . '..' . $ds . 'qs30';
		$new_ini_value .= $ps . $cwd . $ds . 'qs30' . $ds . 'include';
		$new_ini_value .= $ps . $cwd . $ds . '..' . $ds . 'qs30' . $ds . 'include';
		$new_ini_value .= $ps . $cwd . $ds . '..' . $ds . '..' . $ds . 'qs30' . $ds . 'include';
		$new_ini_value .= $ps . $cwd . $ds . 'qs30' . $ds . 'include' . $ds . 'extensions';
		$new_ini_value .= $ps . $cwd . $ds . 'qs30' . $ds . 'include' . $ds . 'gateways';
		$new_ini_value .= $ps . $cwd . $ds . 'qs30' . $ds . 'include' . $ds . 'shipping' . $ds . 'fedex';
		$new_ini_value .= $ps . $cwd . $ds . 'qs30' . $ds . 'include' . $ds . 'shipping' . $ds . 'ups';
		$new_ini_value .= $ps . $cwd . $ds . 'qs30' . $ds . 'include' . $ds . 'shipping' . $ds . 'usps';
		$new_ini_value .= $ps . $cwd . $ds . 'include';
		$new_ini_value .= $ps . $cwd . $ds . 'include' . $ds . 'extensions';
		$new_ini_value .= $ps . $cwd . $ds . 'include' . $ds . 'gateways';
		$new_ini_value .= $ps . $cwd . $ds . 'include' . $ds . 'shipping' . $ds . 'fedex';
		$new_ini_value .= $ps . $cwd . $ds . 'include' . $ds . 'shipping' . $ds . 'ups';
		$new_ini_value .= $ps . $cwd . $ds . 'include' . $ds . 'shipping' . $ds . 'usps';
		
		if($_isAdmin){
			$new_ini_value .= $ps . $cwd . $ds . 'cp' . $ds . 'include';
			$new_ini_value .= $ps . $cwd . $ds . 'cp' . $ds . 'include' . $ds . 'extensions';
			$new_ini_value .= $ps . $cwd . $ds . 'cp' . $ds . 'include' . $ds . 'reports';
		}

		$this->includePath = $new_ini_value;
		
		$inivalue = ini_get('include_path');
		
		$new_ini_value .= $ps . $inivalue;
		
        ini_set('include_path',$new_ini_value);
		ini_set("include_path", '/home/cddvdstorage/php:' . ini_get("include_path") );
		// ini_set("include_path", '/var/cpanel/php/sessions/ea-php74:' . ini_get("include_path") );
        if($this->debug){
            print 'Include Path: ' . ini_get('include_path') . "\n";
        }
    }    
}

?>