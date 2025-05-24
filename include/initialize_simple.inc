<?php

class Initialize{

    public bool $debug = true;
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
		error_reporting(E_ALL);

		if($_isAdmin){
			if($isReport || $isUtility){
				chdir("../../");
			}
			else{
				chdir("../");
			}
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

		// Load and start session
		$_Session = $_Registry->LoadClass("session");

		//start session
		$this->cnx = $_Session->Start(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME, SESSION_TIMEOUT);

		// Clean up user input in $_REQUEST
		$this->cleanupRequest();

		// load general database class
		$_DB = $_Registry->LoadClass("db");

		// load general database class
		$_Config = $_Registry->LoadClass("config");
		$_Config->read(NULL,true);

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
		
        foreach ($_REQUEST as $k => $v) {
            if ($_isAdmin && $k == "editFile") {
                continue;
            }
            if (is_array($_REQUEST[$k])) {
                foreach ($_REQUEST[$k] as $k2 => $v2) {
                    if (is_array($_REQUEST[$k][$k2])) {
                        foreach ($_REQUEST[$k][$k2] as $k3 => $v3) {
                            if (is_array($_REQUEST[$k][$k2][$k3])) {
                                foreach ($_REQUEST[$k][$k2][$k3] as $k4 => $v4) {
                                    $_REQUEST[$k][$k2][$k3][$k4] = $this->escape_string($v4);
                                }
                            } else {					
                                $_REQUEST[$k][$k2][$k3] = $this->escape_string($v3);
                            }
                        }
                    } else {					
                        $_REQUEST[$k][$k2] = $this->escape_string($v2);
                    }
                }
            } else {
                $_REQUEST[$k] = $this->escape_string($v);
            }
        }
	}

	//---------------------------------------------------------------
	function escape_string($string){
		
		global $_isAdmin,$_Common;
	
		if(!$_isAdmin){
			$string = trim(preg_replace("/<script/i","",$string));
			$string = trim(str_replace(">","",$string));
			$string = trim(str_replace(";","",$string));
			$string = trim(preg_replace("/[\"\\\&\`\;\*\^\!\(\)\{\}\<\>\n\r]/","",$string));
		}

        if ($this->cnx && function_exists('mysqli_real_escape_string')) {
            $string = mysqli_real_escape_string($this->cnx, $string);
        } else {
            // Fallback if no connection or function somehow not available
             trigger_error('escape_string in Initialize (simple) class called without a valid mysqli connection or function missing.', E_USER_WARNING);
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
        if($this->debug){
            print 'Include Path: ' . ini_get('include_path') . "\n";
        }
    }    
}

?>