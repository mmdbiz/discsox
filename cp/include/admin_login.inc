<?php
//VersionInfo:Version[3.0.1]


	// customer only login

class Admin_login{

    var $debug = false;
    var $username = "";
    var $password = "";
    // general rights for all users, these are ignored on rights check
	var $rights = array("login" => 1,
						"config.welcome" => 1,
						"news" => 1,
						"welcome" => 1);
	
	// ----------------------------------------------------------------
	function __construct(){
	}
	
	// ----------------------------------------------------------------
    function checkLogin(){
		
		global $_adminFunction;
		global $_CF;
		global $_Common;
		$sessid = session_id();
		if($this->debug){
              $_Common->sendMessage("The current session ID: ". $sessid);
          }
                       
		if(empty($_REQUEST['user']) && empty($_REQUEST['pass'])){
			if(!empty($_SESSION['aid'])){
				$aid = $_SESSION['aid'];
				
                if(!empty($_COOKIE['aid'])){
					if($_COOKIE['aid'] == md5($aid)){
						// check user level for this specific function
						if(!empty($_SESSION['rights'])){
							//$rArray = split(",",$_SESSION['rights']);
							$rArray = preg_split("/,/",$_SESSION['rights']); //marcello fix deprecated
							foreach($rArray as $i=>$right){
								$this->rights[$right] = 1;
							}
						}
						if(empty($this->rights[$_adminFunction])){
							$f = ucwords($_adminFunction);
							$_Common->printErrorMessage("Unauthorized","Sorry, you are not authorized to run the $f function.");
							exit;
						}
						return true;
					}
					else{
						$_SESSION['aid'] = NULL;
						unset($_SESSION['aid']);
						setcookie ("aid", "", time() - 3600);
						$_Common->sendMessage("Please Login");
						$_Common->redirect("login.php");
						exit;
					}					
                }
                else{
                    $_SESSION['aid'] = NULL;
					unset($_SESSION['aid']);
                    setcookie ("aid", "", time() - 3600);
					$_Common->sendMessage("Please Login");
					$_Common->redirect("login.php");
                    exit;
                }
			}
		}

	
          if($this->debug){
              $_Common->sendMessage("username and PWD: ". $_REQUEST['user'] ." " . $_REQUEST['pass']);
          }
		
		if(empty($_REQUEST['user'])){
			$this->logout();
			$_Common->sendMessage("Login username is required.");
			$_Common->redirect("login.php");
			exit;
		}
		if(empty($_REQUEST['pass'])){
			$this->logout();
			$_Common->sendMessage("Login password is required.");
			$_Common->redirect("login.php");
			exit;
		}

		// Clean up the user input and remove bad characters
		$this->username = $_Common->cleanUserInput($_REQUEST['user']);
		$this->password = $_Common->cleanUserInput($_REQUEST['pass']);

		// Run the query
		$record = array();
		$this->getAdminRecord($record);

		if(count($record) > 0){
			if($this->debug){
              $_Common->sendMessage("we found a record ");
            }
            if(isset($record['uid'])){
                $loginData['aid'] = trim($record['uid']);
            }
            else{
				$this->logout();
				$_Common->sendMessage("User ID field not found in login record?");
				$_Common->redirect("login.php");
				exit;
            }
            $loginData['rights'] = "";
           	if(!empty($record['rights'])){
				$loginData['rights'] = $record['rights'];
			}
            
            if(isset($record['title'])){
                $loginData['title'] = trim($record['title']);
            }
            elseif(!isset($record['title'])){
                $loginData['title'] = "admin";
            }
            if(!isset($record['welcome_name'])){
                $loginData['welcome_name'] = "Admin";
            }
            else{
                $loginData['welcome_name'] = trim($record['welcome_name']);
            }
        }
		else{
			$this->logout();
			$_Common->sendMessage("Invalid Login Information");
			$_Common->redirect("login.php");
			exit;
		}
		if($this->debug){
              $_Common->sendMessage("The record for: " .  $loginData['welcome_name']);
            }
		$hours = doubleval(SESSION_TIMEOUT) * 3600;
		$loginData['aid'] .= $sessid;
        setcookie("aid", md5($loginData['aid']), time() + $hours);

		foreach($loginData as $key=>$value){
            $_SESSION[$key] = $value;
        }

		$_Common->redirect("welcome.php");
		return true;
		
	}
	
	// -------------------------------------------------------------------
    function getAdminRecord(&$record){
		global $_DB;
		$username = $this->username;
		$password = md5($this->password);
		$sql = "SELECT * FROM users WHERE username = '$username' AND password = '$password' LIMIT 1";
		$rs = $_DB->execute($sql);
		if($_DB->numrows($rs) > 0){
			$record = $_DB->fetchrow($rs, "ASSOC");
		}
    }
	
    // -------------------------------------------------------------------
    function logout(){
		if(!empty($_SESSION['aid'])){
	        $_SESSION['aid'] = NULL;
			unset($_SESSION['aid']);
		}
        setcookie ("aid", "", time() - 3600);
    }
	
}

?>