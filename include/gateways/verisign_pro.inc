<?php

class Verisign_pro {
	
	var $debug = false;
	var $gatewayName = 'verisign_pro';
	var $cfg = array();
	var $params = array();
	var $results = array();
	var $currency = "USD";
	var $declineMessage = null;
	
	var $test_host = "test-payflow.verisign.com";
	var $host = "payflow.verisign.com";

    var $pfProBinaryPath = "./pfpro";
    var $pfProLibPath = "./lib";
    var $pfProCertPath = "./certs";
	
	// --------------------------------------------------------------
	function __construct(){
		
		global $cart;
		global $_Totals;
		
		if(empty($_Totals) && !empty($cart->totals)){
			$_Totals = $cart->totals;
		}

	}
	// --------------------------------------------------------------
	function processPayment(){
		
		global $_Common;
		global $ip_address;
		global $_Totals;
		
		$cfg = $this->readConfig($this->gatewayName);
		if(count($cfg) == 0){
			$_Common->printErrorMessage("Payment Module Error",
										"Could not read configuration information for " . $this->gatewayName . " payment module");
		}

		$this->cfg = $cfg;
		if($this->cfg['status']['debug'] == 'true'){
			$this->debug = true;	
		}
		if($this->debug){
			$_Common->debugPrint($cfg,"Config Settings:");
		}

		$amount = number_format($_Totals['grandtotal'],2);
		$cardNum = $_SESSION['card_number'];
		$expMonth = $_SESSION['expire_month'];
		$expYear = substr($_SESSION['expire_year'],-2);
		$bname = $_SESSION['billaddress_firstname'] . " " . $_SESSION['billaddress_lastname'];
		$bstreet = $_SESSION['billaddress_addr1'];
		$bzip = $_SESSION['billaddress_postalcode'];
		$email = $_SESSION['billaddress_email'];
		$cvv2 = $_SESSION['cvv2'];

		$host = $this->host;
		if($this->debug || strtolower($this->cfg['fields']['test_mode']) == 'true'){
			$host = $this->test_host;
		}
		
		$tx = new pfpro($cfg['fields']['username'],$cfg['fields']['password'],$cfg['fields']['partner'],$host);

		$tx->pfProBinaryPath = $this->pfProBinaryPath;
		$tx->pfProLibPath = $this->pfProLibPath;
		$tx->pfProCertPath = $this->pfProCertPath;
		$tx->debug = $this->debug;	

		// AVS 
		$tx->AVS($bstreet,$bzip);
		
		if(strtolower($cfg['fields']['transaction_type']) == "sale"){
			$tx->sale($amount,$cardNum,$expMonth,$expYear,$bname,$email,$cvv2);	
		}
		else{
			$tx->authorize($amount,$cardNum,$expMonth,$expYear,$bname,$email,$cvv2);
		}

		// process transaction - send to Verisign 
		$tx->process();

		
		if($this->debug){
			$_Common->debugPrint($tx->result,"Response from Verisign");	
		}

		if(empty($tx->result)){
			$_Common->printErrorMessage($this->gatewayName . " Payment Module Error","There was no response from the verisign request.");
		}
		else{
			// parse Results
			if($tx->result['RESULT'] == "0") {
				return true;
			}
			else{
				if(isset($tx->result['RESPMSG'])){
					$flds = explode(':',$tx->result['RESPMSG']);
					$this->declineMessage = $flds[0];
				}
				else{
					$this->declineMessage = "Transaction Declined";
				}
				return false;
			}
		}
	}	
	
	//-----------------------------------------------------
	function readConfig(){
		
		global $_Common;
		global $_DB;
		
		$gateway = $this->gatewayName;
		
		$records = $_DB->getRecords("SELECT payment_gateways.*,payment_gateway_details.*
									 FROM payment_gateways
									 LEFT JOIN payment_gateway_details ON payment_gateways.pgid = payment_gateway_details.pgid
									 WHERE payment_gateways.gateway_name = '$gateway'
									 ORDER BY payment_gateway_details.sequence");
		$cfg = array();
		foreach($records as $j=>$flds){
			$cfg['status']['active'] = $flds['active'];
			$cfg['status']['debug'] = $flds['debug'];
			$cfg['status']['related_payment_form'] = $flds['related_payment_form'];
			$cfg[$flds['section']][$flds['key']] = trim($flds['value']);
		}

		return $cfg;	
	}

	// --------------------------------------------------------------
	// Loads the default values into the database when uploaded
	// --------------------------------------------------------------
	function loadDefaults(){

		global $_DB;

		$gateway = strtolower($this->gatewayName);

		$count = $_DB->getCount('payment_gateways', "WHERE LOWER(gateway_name) = '$gateway'");

		if($count == 0){

			$sql = "INSERT INTO `payment_gateways` ( `gateway_name`, `active`, `debug`, `related_payment_form` ) 
											VALUES ( '$gateway', 'false', 'false', 'credit_card.html')";

			$_DB->execute($sql);
			$pgid = $_DB->getInsertID('payment_gateways','pgid');
			
			if(!empty($pgid)){

				$details['username'] = '';
				$details['password'] = '';
				$details['vendor'] = '';
				$details['partner'] = 'Verisign';
				$details['transaction_type'] = 'Sale';
				$details['test_mode'] = 'false';

				$counter = 0;
				foreach($details as $key=>$value){
					$sql = "INSERT INTO `payment_gateway_details` (`pgid`,`section`,`sequence`,`key`,`value`)
														   VALUES ('$pgid','fields','$counter','$key','$value')";
					$_DB->execute($sql);
					$counter++;
				}
				
				$sql = "INSERT INTO `help`  (`section`, `section_help`, `key`, `key_help`) VALUES 
										('$gateway', 'These are the Verisign XMLPay Gateway parameters', '', ''),
										('$gateway', '', 'vendor', 'Set to the same as your Verisign Login'),
										('$gateway', '', 'partner', 'This is typically Verisign'),
										('$gateway', '', 'transaction_type', 'Sale or Authorization'),
										('$gateway', '', 'username', 'Your Verisign username'),
										('$gateway', '', 'password', 'Your Verisign password'),
										('$gateway', '', 'test_mode', 'Run in test mode?')";
				$_DB->execute($sql);
			}
		}
	}
}

//---------------------------------------------------------------------
class pfpro { 
    
    var $pfProBinaryPath;
    var $pfProLibPath;
    var $pfProCertPath;
    var $debug = false;
    var $host; 
    var $port; 
    var $timeout; 
    var $proxyaddress; 
    var $proxyport; 
    var $proxyuser; 
    var $proxypassword; 
     
    var $transaction = Array();
    var $result = Array();

    // Constructor 
    function __construct($user,  
                   $pwd, 
                   $partner = "VeriSign",  
                   $host = "test-payflow.verisign.com",  
                   $port = 443,  
                   $timeout = 30,  
                   $proxyaddr = null,  
                   $proxyport = null,  
                   $proxyuser = null,  
                   $proxypwd = null) {

        /* case-sensitive. Use your login for this parameter. 
           In future releases you will be able to use this parameter to 
           create multiple users for a single account. */ 
        $this->transaction['USER']    = $user;
        $this->transaction['VENDOR']    = $user;
        /* Case-sensitive password */ 
        $this->transaction['PWD']     = $pwd; 
        /* This field is case-sensitive. Your partner ID is provided to 
           you by the authorized VeriSign Reseller who signed you up for 
           the Payflow Pro service. If you signed up yourself, use 
           VeriSign. */ 
        $this->transaction['PARTNER'] = $partner; 

        $this->host = $host; 
        $this->port = $port; 
        $this->timeout = $timeout; 
        $this->proxyaddress = $proxyaddr; 
        $this->proxyport = $proxyport; 
        $this->proxyuser = $proxyuser; 
        $this->proxypassword = $proxypwd; 
    } 


    /** 
     * @return void 
     * @param amount float 
     * @param card_no int 
     * @param exp_month int 
     * @param exp_year int 
     * @desc Charge and settle a transaction using a credit card. 
     */ 
    function sale($amount, $card_no, $exp_month, $exp_year, $bName, $email, $csc) { 
        $this->transaction['TRXTYPE'] = "S"; 
        $this->transaction['TENDER'] = "C"; 
        $this->transaction['AMT'] = sprintf("%.2f", $amount); 
        $this->transaction['ACCT'] = preg_replace("/[^0-9]/","",$card_no); 
        $this->transaction['EXPDATE'] = $exp_month . substr($exp_year,-2);
        $this->transaction['CVV2'] = preg_replace("/[^0-9]/","",$csc);
        $this->transaction["NAME[".strlen($bName)."]"] = $bName;
        $this->transaction["EMAIL"] = $email;
    } 
     

    /** 
     * @return void 
     * @param amount float 
     * @param card_no int 
     * @param exp_month int 
     * @param exp_year int 
     * @desc Authorize a credit card for later settlement. 
     */ 
    function authorize($amount, $card_no, $exp_month, $exp_year, $bName, $email, $csc) { 
        $this->transaction['TRXTYPE'] = "A"; 
        $this->transaction['TENDER'] = "C"; 
        $this->transaction['AMT'] = sprintf("%.2f", $amount); 
        $this->transaction['ACCT'] = preg_replace("/[^0-9]/","",$card_no); 
        $this->transaction['EXPDATE'] = $exp_month . substr($exp_year, -2);
        $this->transaction['CVV2'] = preg_replace("/[^0-9]/","",$csc);
        $this->transaction["NAME[".strlen($bName)."]"] = $bName;
        $this->transaction["EMAIL"] = $email;
    } 
     
     
    /** 
     * @return void 
     * @param PNREF string 
     * @param amount float 
     * @desc Request a settlement from a previous authorization request. 
             Optional amount to specify a lower or higher (additional 
             charges apply) amount 
     */ 
    function capture($PNREF, $amount = "") { 
        if ($amount) { 
            // Specify lower amount to capture if supplied 
            $this->transaction['AMT'] = $amount;     
        } 
        $this->transaction['TRXTYPE'] = "D"; 
        $this->transaction['TENDER'] = "C"; 
        $this->transaction['ORIGID'] = trim($PNREF); 
    } 
     
     
    /** 
     * @return void 
     * @param PNREF string 
     * @param amount float 
     * @param card_no int 
     * @param exp_month int 
     * @param exp_year int 
     * @desc Issue a credit. Either using original PNREF or a credit card 
     */ 
    function credit($PNREF = "",  
                    $amount = "",  
                    $card_no = "",  
                    $exp_month = "",  
                    $exp_year = "") { 
        if (!$PNREF && !$card_no) { 
            print "You must supply either a card no or original 
                   transaction PNREF to issue a credit"; 
            return 0;     
        } 
        if ($amount) { 
            // Specify lower amount to capture if supplied 
            $this->transaction['AMT'] = $amount;     
        } 
        if ($PNREF) { 
            $this->transaction['ORIGID'] = trim($PNREF); 
        } elseif ($card_no) { 
            $this->transaction['ACCT'] = preg_replace("/[^0-9]/","",$card_no); 
            $this->transaction['EXPDATE'] = $exp_month . substr($exp_year, -2); 
        } 
        $this->transaction['TRXTYPE'] = "C"; 
        $this->transaction['TENDER'] = "C";         
    } 

     
    /** 
     * @return void 
     * @param PNREF string 
     * @desc A void prevents a transaction from being settled. A void 
             does not release the authorization (hold on funds) on the 
             cardholder account 
     */ 
    function void_sale($PNREF) { 
        $this->transaction['TRXTYPE'] = "V"; 
        $this->transaction['TENDER'] = "C"; 
        $this->transaction['ORIGID'] = trim($PNREF); 
    } 
     
     
    /** 
     * @return void 
     * @param avs_address string 
     * @param avs_zip int 
     * @desc Optional, used for AVS check (Address Verification Service) 
     */ 
    function AVS($avs_address = "", $avs_zip = "") { 
        $this->transaction["STREET[".strlen($avs_address)."]"] = $avs_address; 
        $this->transaction['ZIP'] = preg_replace("/[^0-9]/","",$avs_zip); 
    } 
     
     
    function comments($comment1 = "", $comment2 = "") { 
        $this->transaction["COMMENT1[".strlen($comment1)."]"] = $comment1; 
        $this->transaction["COMMENT2[".strlen($comment2)."]"] = $comment2; 
    } 
     

    /** 
     * @return array 
     * @desc Process the transaction. Result contains the response from Verisign. 
     */ 
    function process() { 
    
		global $_Common;

		putenv("PFPRO_CERT_PATH=" . $this->pfProCertPath);
		putenv("LD_LIBRARY_PATH=" . $this->pfProLibPath);

	    if($this->debug){
		    $_Common->debugPrint($this->transaction,"Verisign Parameters");
		}

        if(function_exists('pfpro_init')){
			pfpro_init();
			$this->result = pfpro_process($this->transaction, 
										$this->host, 
										$this->port, 
										$this->timeout, 
										$this->proxyaddress, 
										$this->proxyport, 
										$this->proxyuser,  
										$this->proxypassword); 
			pfpro_cleanup();
		}
		else{
			
			$binary = $this->pfProBinaryPath;
			$host = $this->host;
			$port = $this->port;
			$params = array();
			foreach($this->transaction as $key=>$value){
				$str = "$key=$value";
				$str = str_replace('-',' ',$str);
				$str = str_replace(';',' ',$str);
				$str = str_replace('&',' ',$str);
				$params[] = $str;
			}
			$cmd = join('&',$params);
			$cmd = $str = str_replace(' ','+',$cmd);
			
	        if($this->debug){
		        $_Common->debugPrint($cmd,"Verisign Request");
		    }
						
	        $result = exec("$binary $host $port " . '"' . $cmd . '"', $resultArry, $exit_value);
	        
	        if($this->debug){
		        $_Common->debugPrint($result,"Verisign Results, Exit: $exit_value");
		    }
	        
	        if($result){
				$valArray = explode('&', $result);
				foreach($valArray as $val) {
					$valArray2 = explode('=', $val);
					$this->result[$valArray2[0]] = $valArray2[1];
				}
			}
	    }
    } 
} // end pfpro class 

?>