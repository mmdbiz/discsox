<?php
//VersionInfo:Version[3.0.1]

//error_reporting(E_ALL & ~E_DEPRECATED);
//error_reporting(E_ALL & ~E_NOTICE);
error_reporting(E_ALL);
//error_reporting(0);

//Set up the session hander to point to the database
//and start the session handler up

//Cookies ONLY. Needed for security.
//note: php4.3.0+ only

if(!isset($_SESSION)){  
    // Following is required for PCI - marcello
    // Session ID cannot be passed through URLs
    ini_set('session.use_only_cookies',1);
    
    // Prevents javascript XSS attacks aimed to steal the session ID
    ini_set('session.cookie_httponly',1);
    
    // Uses a secure connection (HTTPS) if possible
    ini_set('session.cookie_secure', 1);
    
    //note: use_trans_sid defaults to 0
    //ini_set('session.use_trans_sid',0);
    
    ini_set('session.bug_compat_42', 1);
    ini_set('session.bug_compat_warn',1);
}

/*
	//Create new object of class
	global $g_Session;
	$g_Session = new Session();

	// Start the session
	$g_Session->Start("127.0.0.1","bob","bobpass","bobdb");
*/

class Session
{
	var $ses_class = null;
	var $m_IsStarted = false;

	// ----------------------------Marcello-------------------------------
//	function __construct(){ //fixed 3/24/21 marcello
//	}
		
	/**
		Setup session with database storage
 
		@return boolean true if handler was set and session started 
		@param string $hostName 
		@param string $userName 
		@param string $password 
		@param string $database
		@param int	$sessionTimeoutInMinutes 

	*/
	
	function Start($hostName,$userName,$password,$database,$sessionTimeoutInMinutes = 30){
		$isOk = false;
		
		//Change the save_handler to use the class functions 
		//Create the session handler
		$ses_class = new MySessionHandler();
		
		//give session handler the info it needs
		$ses_class->hostName = $hostName;
		$ses_class->userName = $userName;
		$ses_class->password = $password;
		$ses_class->database = $database;
		$ses_class->sessionTimeout = "-$sessionTimeoutInMinutes minutes";

		if(!isset($_SESSION)){  
			//Change the save_handler to use the class functions 
			$isOk = session_set_save_handler(array(&$ses_class, '_open'),
									array(&$ses_class, '_close'),
									array(&$ses_class, '_read'),
									array(&$ses_class, '_write'),
									array(&$ses_class, '_destroy'),
									array(&$ses_class, '_gc'));
		}

		//		echo(" isOk: " . $isOk);
		//$this->cnx = $ses_class->cnx;								
		
		if($isOk){
			// for returns from processors
			//$cartid = NULL;
			$cartid = [];
			if(!empty($_REQUEST['cart_id'])){
				$cartid = trim($_REQUEST['cart_id']);
				session_id($cartid);
			}
			elseif(!empty($_REQUEST['cartid'])){
				$cartid = trim($_REQUEST['cartid']);
				session_id($cartid);
			}
			elseif(!empty($_REQUEST['cartId'])){
				$cartid = trim($_REQUEST['cartId']);
				session_id($cartid);
			}
			
			if(session_status() == PHP_SESSION_NONE){ 
				$cartid = [];
				//echo(" session not started ");
				//session has not started 
				session_start($cartid);	 
			}
			
			//save a ref to the object may not strictly be needed
			$this->ses_class = $ses_class;
			
			//and set flag in case we should need to check later
			$this->m_IsStarted = true;
		}
		
		return $ses_class->cnx;
	}
}


/*
Table for holding session sis expected to be in the following form

CREATE TABLE sessions (
   ses_id varchar(32) NOT NULL default '',
   ses_time int(11) NOT NULL default '0',
   ses_start int(11) NOT NULL default '0',
   ses_value text NOT NULL,
   PRIMARY KEY  (ses_id)
) TYPE=MyISAM;

*/

//note: SessionHandler depends on the existance of mysql.
class MySessionHandler // Marcello
{
    // Define the mysql table you wish to use with this class, this table MUST exist.
    var $ses_table = 'sessions';

    // Change to 'Y' if you want to connect to a db in  the _open function
    var $db_con = 'Y';

	//how long to save a session without any activity in it.
	//thus this becomes the lifetime of a cart without any save
	//type options. 
	var $sessionTimeout = '-30 minutes';
	
	//The database connection values these must be set before  this is called
	var $hostName;
	var $userName;
	var $password;
	var $database;
	var $cnx;
	
	function __construct(){
		if(!empty($_SERVER['REMOTE_ADDR'])){
            $this->ip = $_SERVER['REMOTE_ADDR'];
        }
		elseif(!empty($_SERVER['REMOTE_HOST'])){
			$this->ip = $_SERVER['REMOTE_HOST'];
		}
	}

    // Create a connection to a database
    function db_connect() {

		//note: do we realy want a pconnect over a connect here
        $mysql_connect = mysqli_connect ($this->hostName,
                                          $this->userName,
                                          $this->password);
        //$mysql_connect = mysqli_connect ( "127.0.0.1", $this->userName, $this->password);
//        $mysql_db = mysqli_select_db ($mysql_connect, $this->database);

        if (!$mysql_connect) {
            return FALSE;
        } else {
        	 $mysql_db = mysqli_select_db ($mysql_connect, $this->database);
        	 if (!$mysql_db) {
             return FALSE;
           } else { 
			       $this->cnx = $mysql_connect;
            return TRUE;
           }
        }
        
//        if (!$mysql_connect || !$mysql_db) {
//            return FALSE;
//        } else {
//			$this->cnx = $mysql_connect;
//            return TRUE;
//        }
    }

    // Open session, if you have your own db connection code, put it in here!
    function _open($path, $name) {
        if ($this->db_con == 'Y') {
            $this->db_connect();
        }
        return TRUE;
    }

    // Close session
    function _close() {
        //This is used for a manual call of the session gc function
        $this->_gc(0);
        return TRUE;
    }

    // Read session data from database
    function _read($ses_id) {
        $session_sql = 'SELECT 
        * FROM ' . $this->ses_table
                     . " WHERE ses_id = '$ses_id'";
        $session_res = @mysqli_query($this->cnx, $session_sql);
        if (!$session_res) {
            return '';
        }
        $session_num = @mysqli_num_rows ($session_res);
        if ($session_num > 0) {
            $session_row = mysqli_fetch_assoc ($session_res);
            $ses_data = $session_row['ses_value'];
            return $ses_data;
        }else{
            return '';
        }
    }

    // Write new data to database
    function _write($ses_id, $data) {
		
		$host = null;
		
//		if(function_exists('gethostbyaddr')){
//			$host = strtolower(@gethostbyaddr($this->ip));
//			if(strstr($host,'googlebot') || strstr($host,'crawl') || strstr($host,'slurp') || strstr($host,'msnbot') || strstr($host,'scanalert.com')){
//				return true;	
//			}
//		}
		if(trim($host) == ""){
			$host = $this->ip;	
		}


        $session_sql = 'UPDATE ' . $this->ses_table
                     . " SET ses_time='" . time()
                     . "', ses_value='$data',ses_fingerprint='$host' WHERE ses_id='$ses_id'";
                     
        $session_res = @mysqli_query ($this->cnx, $session_sql);

        if (!$session_res) {
            return FALSE;
        }
        
        if (mysqli_affected_rows ($this->cnx)) {
            return TRUE;
        }

        $session_sql = 'INSERT INTO ' . $this->ses_table
                     . ' (ses_id, ses_time, ses_start, ses_value, ses_fingerprint)'
                     . " VALUES ('$ses_id', '" . time()
                     . "', '" . time() . "', '$data', '$host')";
                     
        $session_res = @mysqli_query ($this->cnx, $session_sql);
        if (!$session_res) {
            return FALSE;
        }else{
            return TRUE;
        }
       
    }

    // Destroy session record in database
    function _destroy($ses_id) {
        $session_sql = 'DELETE FROM ' . $this->ses_table
                     . " WHERE ses_id = '$ses_id'";
        $session_res = @mysqli_query ($this->cnx, $session_sql);
        if (!$session_res) {
            return FALSE;
        }else{
            return TRUE;
        }
    }

	
    // Garbage collection, deletes old sessions
    function _gc($life) {
        
        global $_Common;
        
        $ses_life = strtotime($this->sessionTimeout);
		$session_res = null;
		
		$sql = 'SELECT * FROM ' . $this->ses_table . " WHERE ses_time < $ses_life";
		$rs = @mysqli_query ($this->cnx, $sql);
		$rows = @mysqli_num_rows($rs);
		
		if($rows > 0){
			while($row = @mysqli_fetch_array($rs, MYSQL_ASSOC)){
				$ses_id = $row['ses_id'];
				$ses_data = trim($row['ses_value']);
				$this->checkAbandonedCarts($ses_id,$ses_data);
			}
			$session_sql = 'DELETE FROM ' . $this->ses_table . " WHERE ses_time < $ses_life";
			$session_res = @mysqli_query ($this->cnx, $session_sql);
			
			@mysqli_query($this->cnx, "OPTIMIZE TABLE sessions");
			
			//$_Common->optimizeTables('session');
		}

        if(!$session_res) {
            return FALSE;
        }
        else{
            return TRUE;
        }
    }
    
    // checks abandoned carts and logs some basic data
    function checkAbandonedCarts($ses_id,$ses_data){
		
		global $_Common;

		// check for username/email address
		$sessionData = array();
		$sessionData = $this->unserialize_session_data($ses_data);
		
		$username = null;
		if(isset($sessionData['billaddress_firstname']) && isset($sessionData['billaddress_lastname'])){
			$username = trim($sessionData['billaddress_firstname']) . " " . trim($sessionData['billaddress_lastname']);
		}
		$email_address = null;
		if(isset($sessionData['billaddress_email'])){
			$email_address = trim($sessionData['billaddress_email']);
		}
	
		// check for a cart
		$sql = "SELECT cartid,sessionid,last_page,quantity,price
				FROM carts WHERE sessionid = '$ses_id'";
		$rs = @mysqli_query ($this->cnx, $sql);
		$rows = @mysqli_num_rows($rs);
		
		if($rows > 0){
			
			$abandonData = array();
			$cartids = array();
			
			while($row = @mysqli_fetch_array($rs, MYSQL_ASSOC)){

				$cID = $row['cartid'];
				$cartids[$cID] = $row;
				$qty = $row['quantity'];
				$price = $this->calculateQuantityPrice($row['price'],$qty);
				$row['linetotal'] = number_format($price * $qty,2);

				// check option prices in details
				$detailSQL = "SELECT price,type FROM cart_details WHERE cartid = '$cID'";
				$rs1 = @mysqli_query ($this->cnx, $detailSQL);
				while($dRow = @mysqli_fetch_array($rs1, MYSQL_ASSOC)){
					if(isset($dRow['type']) && strtolower($dRow['type']) == "option"){
						if($dRow['price'] > 0){
							$row['linetotal'] += ($dRow['price'] * $qty);
						}
					}
					elseif(isset($dRow['type']) && strtolower($dRow['type']) == "setup"){
						if($dRow['price'] > 0){
							$row['linetotal'] += $dRow['price'];
						}
					}
				}
				@mysqli_free_result($rs1);

				if(isset($abandonData[$row['sessionid']])){
					$abandonData[$row['sessionid']]['linetotal'] += $row['linetotal'];
					$abandonData[$row['sessionid']]['number_of_items'] += $row['quantity'];
				}
				else{
					$abandonData[$row['sessionid']]['linetotal'] = $row['linetotal'];
					$abandonData[$row['sessionid']]['number_of_items'] = $row['quantity'];
					$abandonData[$row['sessionid']]['last_page'] = $row['last_page'];
				}
			}
			@mysqli_free_result($rs);
			unset($row);

			if(count($cartids) > 0){

				$date = date("Y-m-d");
				foreach($abandonData as $sid=>$data){
					
					$sql = "SELECT * FROM 'abandoned_carts' WHERE sid = '$sid'";
					$rs2 = @mysqli_query ($this->cnx, $sql);
					$count = @mysqli_num_rows($rs2);
					@mysqli_free_result($rs2);
					
					if($count == 0){
						$lPage = $data['last_page'];
						$numItems = $data['number_of_items'];
						$cTotal = number_format($data['linetotal'],2);
						$sql = "INSERT INTO abandoned_carts (`sid`,`date`,`number_of_items`,`cart_total`,`last_page`,`email_address`,`username`)
													 VALUES ('$sid','$date','$numItems','$cTotal','$lPage','$email_address','$username')";
						$rs3 = @mysqli_query ($this->cnx, $sql);
						@mysqli_free_result($rs3);
					}
				}

				$idList = "'" . join("','",array_keys($cartids)) . "'";
				$cSql = "DELETE FROM carts WHERE cartid IN ($idList)";
				$rs4 = @mysqli_query ($this->cnx, $cSql);
				@mysqli_free_result($rs4);
				
				$dSql = "DELETE FROM cart_details WHERE cartid IN ($idList)";
				$rs5 = @mysqli_query ($this->cnx, $dSql);
				@mysqli_free_result($rs5);
				
				@mysqli_query ($this->cnx, "OPTIMIZE TABLE carts");
				@mysqli_query ($this->cnx, "OPTIMIZE TABLE cart_details");
				
			}
			//$_Common->optimizeTables('cart');
		}
	}
	
	function unserialize_session_data( $serialized_string ) {
		$variables = array(  );
		$a = preg_split( "/(\w+)\|/", $serialized_string, -1, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE );
		for( $i = 0; $i < count( $a ); $i = $i+2 ) {
			$variables[$a[$i]] = unserialize( $a[$i+1] );
		}
	return( $variables );
	}

	// --------------------------------------------------------------
	function calculateQuantityPrice($price,$quantity = 1){

		if(strstr($price,":")){
		
			$qtyPrices = explode(",",$price);

			foreach($qtyPrices as $i=>$rates){
				
				list($qty,$prc) = explode(":",$rates);
				//$this->debugPrint("$qty,$prc","Quantity Prices");

				if(strstr($qty,"-")){
					@list($low,$high) = explode("-",$qty);
					if($low == ""){
						$low = 1;
					}
					if($high == "" || $high == "+"){
						$high = 99999999;
					}
					if($quantity >= $low && $quantity <= $high){
						$price = number_format($prc,2,'.',',');
					}
				}
				else{
					if($quantity >= $qty){
						$price = number_format($prc,2,'.',',');
					}
				}
			}
		}

		//$this->debugPrint($price,"calculateQuantityPrice result");

	return $price;
	}
}
?>