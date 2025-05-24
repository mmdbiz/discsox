<?php
//VersionInfo:Version[3.0.1]


	// customer only login

class Login{

    public bool $debug = false;
    public $message = null;
    public string $username = "";
    public string $password = "";
    public $require_login;
    public $require_password;
    public string $userFrmFld = "user";
    public string $passFrmFld = "pass";
    public string $userEmailFld = "user_email";
    public $results;
    public array $vars = [];

	// ----------------------------------------------------------------
    function __construct(){
		
		global $_CF;
		global $_Registry;
		
		$cart = $_Registry->LoadClass('Cart');
		$cart->updateLastPage("login.php");

		if(empty($_SESSION['logging_in_from'])){
			if(!empty($_SERVER['HTTP_REFERER']) && $_SERVER['HTTP_REFERER'] != $_SERVER['PHP_SELF']){
				$_SESSION['logging_in_from'] = $_SERVER['HTTP_REFERER'];
			}
			else{
				$_SESSION['logging_in_from'] = $_CF['basics']['web_site_url'];
			}
		}
		elseif(stristr($_SESSION['logging_in_from'],"welcome.php")){
			$_SESSION['logging_in_from'] = $_CF['basics']['web_site_url'];
		}
		
        $this->require_login = $_CF['login']['require_login'];
		$this->require_password = $_CF['login']['require_password'];
	}

	// ----------------------------------------------------------------
    function checkLogin(){	

		global $_Common, $_CF;

		if(isset($_REQUEST['forgot'])){
			$this->forgotPassword();	
		}
		elseif(isset($_REQUEST['logout'])){
			session_destroy();
			$_SESSION['login_message'] = "You have been logged out";
			//$_Common->redirect("login.php");
			//$_Common->redirect($_CF['basics']['web_site_url']);
			$_Common->redirect($_CF['basics']['home_page_name']);
			exit;
		}
		elseif($_CF['login']['require_login'] && !$this->lookupUser()){
			$_SESSION['login_message'] = $this->message;
			$_Common->redirect("login.php");
			exit;
		}
		elseif(!$this->lookupUser() && !empty($_REQUEST[$this->userFrmFld])){
			$_SESSION['login_message'] = $this->message;
			$_Common->redirect("login.php");
			exit;
		}
	}
	
	// ----------------------------------------------------------------
    function lookupUser(){	

		global $_CF;
		global $_Common;
		global $_DB;
		$sessid = session_id();

		if(!empty($_REQUEST[$this->userFrmFld])){
            $this->username = $_Common->cleanUserInput(trim($_REQUEST[$this->userFrmFld]));
        }
        if(!empty($_REQUEST[$this->passFrmFld])){
            $this->password = $_Common->cleanUserInput(trim($_REQUEST[$this->passFrmFld]));
        }
		// Check the session vars to see if we are already logged in.
        if($this->username == "" && $this->password == ""){
			
			$cid = empty($_SESSION['cid']) ? NULL : $_SESSION['cid'];

			if($cid){
				if(!empty($_COOKIE['cid']) && trim($_COOKIE['cid']) == md5($cid . $sessid)){
					if(!empty($_SERVER['PHP_SELF']) && basename($_SERVER['PHP_SELF']) == "login.php"){
						$this->message = $_CF['login']['please_login_message'];
						return false;
					}
					else{
						return true;
					}
				}
				elseif(!empty($_SESSION['isRegistered']) && $_SESSION['isRegistered'] == 'true'){
					$check = $_DB->getRecord("SELECT ses_cid FROM sessions WHERE ses_id = '$sessid'");
					if(!empty($check['ses_cid']) && $check['ses_cid'] == md5($cid . $sessid)){
						$hours = number_format(SESSION_TIMEOUT,2) * 3600;
						setcookie("cid", $check['ses_cid'], time() + $hours);
						return true;
					}
					else{
						$_SESSION['cid'] = NULL;
						unset($_SESSION['cid']);
						setcookie ("cid", "", time() - 3600);
						$this->message = $_CF['login']['please_login_message'];
						return false;
					}
				}
				else{
					$_SESSION['cid'] = NULL;
					unset($_SESSION['cid']);
					setcookie ("cid", "", time() - 3600);
					$this->message = $_CF['login']['please_login_message'];
					return false;
				}
            }
            else{
                $this->message = $_CF['login']['please_login_message'];
				return false;
			}
		}
		else{
			if(trim($this->username) == ""){
				$this->message = $_CF['login']['username_required_message'];
				return false;
			}
			if(trim($this->password) == "" && $this->require_password){
				$this->message = $_CF['login']['password_required_message'];
				return false; 
			}
		}

		// Run the queries
        $record = array();
        $loginData = array();

		$this->getCustomerRecord($record);

		if($this->debug){
			$_Common->debugPrint($record,"login Record");
		}


		if(count($record) > 0){

			$loginCid = NULL;
            if(isset($record['cid'])){
                $loginCid = md5(trim($record['cid']));
                $record['cid'] = $loginCid;
            }
            else{
				$this->message = $_CF['login']['missing_record_message'];
				return false;
            }
            if(isset($record['firstname']) && isset($record['lastname'])){
				$first = trim($record['firstname']);
				$last = trim($record['lastname']);
                if($first != "" || $last != ""){
                    $record['welcome_name'] = "$first $last";
                }
            }
			elseif(isset($record['company_name']) && $record['company_name'] != ""){
				$record['welcome_name'] = trim($record['company_name']);
			}
        }
        else{
			$this->message = $_CF['login']['missing_record_message'];
			return false;
        }

		$hours = number_format(SESSION_TIMEOUT,2) * 3600;
        setcookie("cid", md5($loginCid . $sessid), time() + $hours);
        
        $record['isRegistered'] = "true";
        foreach($record as $key=>$value){
            $_SESSION[$key] = trim($value);
        }

		$sess_cid = md5($loginCid . $sessid);
		$_DB->execute("UPDATE sessions SET `ses_cid` = '$sess_cid' WHERE `ses_id` = '$sessid'");

        // all done...
		$this->goToLastPage();
		exit;
	
	}
	
	// --------------------------------------------------------------
	function forgotPassword(){
		
		global $_Common,$_CF,$_CR,$_DB;
		
		$this->username = null;
		$this->password = null;
		$this->require_password = false;
		$user_email = null;

		$record = array();

		if(!empty($_REQUEST['user_name'])){
			$this->username = $_Common->cleanUserInput(trim($_REQUEST['user_name']));
			if(is_null($this->username) || trim($this->username) == ""){
				$this->message = $_CF['login']['username_required_message'];
				$_SESSION['login_message'] = $this->message;
				$_Common->redirect("login.php");
				exit;
			}
			$username = $this->username;
			if(strtolower(substr($username,0,5)) == "admin"){
				$this->message = "You cannot retrieve the $username Password";
				$_SESSION['login_message'] = $this->message;
				$_Common->redirect("login.php");
				exit;
			}
			$record = $_DB->getRecord("SELECT `cid`,`username`,`password`,`billaddress_email`
									FROM customers WHERE active = 'true' AND username = '$username' LIMIT 1");
		}
		elseif(!empty($_REQUEST[$this->userEmailFld])){
			$user_email = $_Common->cleanUserInput(trim($_REQUEST[$this->userEmailFld]));
			if(preg_match("/^[_\.0-9a-z-]+@([0-9a-z][-0-9a-z\.]+)\.([a-z]{2,3}$)/i",$user_email,$test)){
				$record = $_DB->getRecord("SELECT `cid`,`username`,`password`,`billaddress_email`
										   FROM customers WHERE active = 'true' AND billaddress_email = '$user_email' LIMIT 1");
			}
			else{
				$this->message = "That is an invalid email address for this account. Please contact the store administrator";
				$_SESSION['login_message'] = $this->message;
				$_Common->redirect("login.php");
				exit;
			}
		}
		
		$username = null;
		$password = null;
		$reset = false;
		
		if(!empty($record['cid']) && !empty($record['billaddress_email'])){
			
			if($_CF['login']['encrypt_password']){
				// reset the password to a random password and email
				if(!empty($record['username'])){
					$username = $record['username'];
				}
				$password = $this->createRandomPassword();
				$cid = $record['cid'];
				$sql = "UPDATE customers SET `password` = MD5('$password') WHERE `cid` = '$cid' LIMIT 1";
				$_DB->execute($sql);
				$reset = true;				
			}
			elseif(!empty($record['password']) && !empty($record['username'])){
				$username = $record['username'];
				$password = trim($record['password']);
			}
			else{
				// create a new username/password for them.
				@list($username,$junk) = explode('@',$record['billaddress_email']);
				
				if($_CF['login']['require_password']){
					$password = $this->createRandomPassword();
					$cid = $record['cid'];
					if($_CF['login']['encrypt_password']){
						$sql = "UPDATE customers SET `username` = '$username', `password` = MD5('$password') WHERE `cid` = '$cid' LIMIT 1";
					}
					else{
						$sql = "UPDATE customers SET `username` = '$username', `password` = '$password' WHERE `cid` = '$cid' LIMIT 1";
					}
					$_DB->execute($sql);
				}
				else{
					$sql = "UPDATE customers SET `username` = '$username' WHERE `cid` = '$cid' LIMIT 1";
					$_DB->execute($sql);
				}
			}

			$custEmailAddress = $record['billaddress_email'];
			$check = array();
			if(preg_match("/^[_\.0-9a-z-]+@([0-9a-z][-0-9a-z\.]+)\.([a-z]{2,3}$)/i",$custEmailAddress,$check)){
				
				global $_Registry;
				$email = $_Registry->loadClass("email");
				
				$to = $custEmailAddress;
				$from = $_CF['email']['store_email_address'];
				$storeName = $_CF['basics']['store_name'];
				$subject = "Your Log In Information for $storeName";
				$loginURL = $_CF['basics']['web_site_url'] . "/login.php";
				
				error_reporting(E_PARSE|E_WARNING);
				ob_start();
				include_once("templates/forgot.email.html");
				$template = ob_get_contents();
				ob_end_clean();
				error_reporting(E_ALL);

				@list($html,$text) = explode("<text>",$template);
				$email->send($from, $to, $subject, $html, $text);
			}
			else{
				$this->message = "There is an invalid email address for this account. Please contact the store administrator";
				$_SESSION['login_message'] = $this->message;
				$_Common->redirect("login.php");
				exit;	
			}

			$this->message = "Your Password has been sent";
			$_SESSION['login_message'] = $this->message;
			$_Common->redirect("login.php");
			exit;

		}
		else{
			$this->message = $_CF['login']['missing_record_message'];
			$_SESSION['login_message'] = $this->message;
			$_Common->redirect("login.php");
			exit;	
		}
	}
	// --------------------------------------------------------------
	function createRandomPassword() {
		$chars = "aBcDeFgHiJkMnOpQrStUvWxYz023456789";
		$i = 0;
		$pass = '' ;
		while ($i <= 7) {
			$num = rand() % 33;
			$tmp = substr($chars, $num, 1);
			$pass = $pass . $tmp;
			$i++;
		}
		return $pass;
	}

    // --------------------------------------------------------------
    function getCustomerRecord(&$record){
		
		global $_CF;
		global $_Common;
		global $_DB;
		
		$username = $this->doEscape($this->username);
		
		if($_CF['login']['encrypt_password']){
			$password = md5($this->password);
		}
		else{
			$password = $this->doEscape($this->password);
		}
		
		if($this->require_password){
			$sql = "SELECT cid,customer_type,discount_rate,is_taxable,billaddress_firstname as firstname,billaddress_lastname as lastname 
					FROM customers WHERE active = 'true' AND username = '$username' AND password = '$password' LIMIT 1";
		}
		else{
			$sql = "SELECT cid,customer_type,discount_rate,is_taxable,billaddress_firstname as firstname,billaddress_lastname as lastname
					FROM customers WHERE active = 'true' AND username = '$username' LIMIT 1";			
		}
		if($this->debug){
			print "<pre>$sql</pre>\n";
		}
		$rs = $_DB->execute($sql);
		if($_DB->numrows($rs) > 0){
			$record = $_DB->fetchrow($rs, "ASSOC");
		}
    }
    
    function doEscape($string){
	
        global $_DB;
        if (isset($_DB) && $_DB->cnx) {
            // Ensure mysqli_real_escape_string is available (it should be as mysqli is used)
            if (function_exists('mysqli_real_escape_string')) {
                $string = mysqli_real_escape_string($_DB->cnx, $string);
            } else {
                // Fallback if mysqli_real_escape_string somehow not available, though unlikely
                $string = addslashes($string);
            }
        } else {
            // Fallback if $_DB or its connection isn't available
            trigger_error('doEscape in Login class called without a valid global DB connection.', E_USER_WARNING);
            $string = addslashes($string);	
        }
        return $string;
	}
    
    
	// -------------------------------------------------------------------
	function goToLastPage(){
		
		global $_CF;
		global $_Common;
		
		$lastPage = "";
		if(!empty($_SESSION['logging_in_from'])){
			$lastPage = $_SESSION['logging_in_from'];
		}
		if(trim($lastPage) != "" && $lastPage != $_SERVER['PHP_SELF']){
			$_Common->redirect($lastPage);
		}
		else{
			$_Common->redirect($_CF['basics']['web_site_url']);
		}
		exit;
	}

	
    // -------------------------------------------------------------------
    function logout(){
        unset($_SESSION['cid']);
        setcookie ("cid", "", time() - 3600);
        session_destroy();
    }
	
}

?>