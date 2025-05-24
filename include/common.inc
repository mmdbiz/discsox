<?php
// ------------------------------------------------------------------
//         Class: Common
//       Purpose: This class has common functions used throughout the code.
// Last Modified: March 08, 2006
//
// ------------------------------------------------------------------
class Common{

	function __construct(){
	
	}

	// --------------------------------------------------------------
	// recursive array trim function
	function array_trim($arr){
		
		$result = array();
		
		foreach($arr as $key=>$value){
			if(is_array($value)){
				$result[$key] = $this->array_trim($value);
			}
			else{
				$result[$key] = trim($value);
			}
		}
		return $result;
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
						$price = $this->format_number($prc);
					}
				}
				else{
					if($quantity >= $qty){
						$price = $this->format_number($prc);
					}
				}
			}
		}

		//$this->debugPrint($price,"calculateQuantityPrice result");

	return $price;
	}
	
	// -------------------------------------------------------------------
	function cleanUserInput($value){
		$value = trim(preg_replace("/[\&\`\;\*\|\^\!\(\)\{\}\<\>\n\r]/","",$value));
		return trim($value);
	}

	// --------------------------------------------------------------
	function debugPrint($data,$title = "",$hide = false){

		if($hide){
			print "<!--\n";	
		}
		if(is_array($data)){
			print "<div align=left><pre><b>$title</b><br>\n";
			print_r($data);
			print "</pre></div>\n";
		}
		else{
			print "<pre><b>$title</b><br>$data</pre>\n";
		}
		if($hide){
			print "-->\n";	
		}
	}
	
	// --------------------------------------------------------------
	function format_number($number,$decimals = 2,$dec_point = ".",$thousands_sep = ""){

		global $_CF;
		$cSymbol = $_CF['basics']['currency_symbol'];

		if(strstr($number,':')){
			$number = $this->calculateQuantityPrice($number,1);
		}

		$number = str_replace(',','',$number);
		$number = str_replace($cSymbol,'',$number);
		$number = floatval($number);

		$number = number_format($number,$decimals,$dec_point,$thousands_sep);
	
	return $number;		
	}
	// legacy format_price
	// --------------------------------------------------------------
	function format_price($number,$forDisplay = false){
		
		global $_CF;
		$cSymbol = $_CF['basics']['currency_symbol'];

		if(strstr($number,':')){
			$number = $this->calculateQuantityPrice($number,1);
		}

		$number = str_replace(',','',$number);
		$number = str_replace($cSymbol,'',$number);
		
		if($forDisplay){
			return $cSymbol . $this->format_number($number,2,".",",");
		}
		else{
			return $this->format_number($number);
		}
	}
	
	// --------------------------------------------------------------
	function GetDirArray($sPath,$extension = "html",$lookInSubdirectories = false){

		global $lookInSubdirectories;
		global $pageDir;
		global $pageIndex;
		
		$validPages = array($extension => "1");

		$currentDir = $sPath;
		$subDir = preg_replace("|$pageDir|","",$currentDir);
		$retVal = array();

		//Load Directory Into Array
		$handle=opendir($sPath);
		while ($file = readdir($handle)){
			$retVal[count($retVal)] = $file;
		}
		//Clean up and sort
		closedir($handle);

		sort($retVal);

		//while(list($key, $val) = each($retVal)){
		foreach ($retVal as $key => $val){

			if($val != "." && $val != ".." && substr($val,0,1) != "_"){
				
				$path = "$sPath/$val";

				if($this->getFileExtension($val,$validPages)){
					$htmlPage = $val;
					if($subDir != ""){
						$pageIndex["$subDir/$htmlPage"] = 1;
					}
					else{
						$pageIndex[$htmlPage] = 1;
					}				
				}
				
				if(is_dir($path) && $lookInSubdirectories){
					$this->GetDirArray($path);
				}
			}
		}
		//echo "got here";
		return $pageIndex;
	}
	// --------------------------------------------------------------
	function getFileExtension($filename,$validPages){
		$pathinfo = pathinfo($filename);
		if(!empty($pathinfo['extension']) && !empty($validPages[$pathinfo['extension']])){
			return true;	
		}
		return false;
	}

	// -------------------------------------------------------------- 
	function getWebSiteUrl(){
		
		global $_CF;
			
		$host = $_SERVER['HTTP_HOST'];
		$tempurl = $_CF['basics']['web_site_url'];
		
		if($tempurl){      
			if(strtolower(substr($tempurl,0,4)) != 'http'){
				$siteurl = "http://$host" . $tempurl;
			}
			else{
				$siteurl = $tempurl;
			}
		}
		else{
			$siteurl = "http://$host";
		}

		return $siteurl;
	}



	// -------------------------------------------------------------------
	// creates the select box for states,provinces,countries
	function loadStateCountry(&$record){

		global $fldProperties;
		global $countries;
		global $states;
		global $provinces;

		$countryValues = array_keys($countries);
		$countryNames = array_values($countries);
		
		$stateValues = array_keys($states);
		$stateNames = array_values($states);

		$provinceValues = array_keys($provinces);
		$provinceNames = array_values($provinces);
		
		$selectedCountry = "US";
		if(!empty($record['billaddress_country'])){
			$selectedCountry = $record['billaddress_country'];
		}
		if(!empty($record['shipaddress_country'])){
			$selectedCountry = $record['shipaddress_country'];
		}

		foreach($record as $key=>$value){
			// make pull-downs where we can
			if($selectedCountry == "CA" && substr($key,-5) == "state"){
				list($select,$selected) = $this->makeSelectBox($key,$provinceNames,$provinceValues,$value);
				$record[$key] = $select;
			}
			elseif($selectedCountry == "US" && substr($key,-5) == "state"){
				list($select,$selected) = $this->makeSelectBox($key,$stateNames,$stateValues,$value);
				$record[$key] = $select;
			}
			elseif($selectedCountry != "US" && $selectedCountry != "CA" && substr($key,-5) == "state"){
				$record[$key] = "None";
			}
			elseif(substr($key,-7) == "country"){
				list($select,$selected) = $this->makeSelectBox($key,$countryNames,$countryValues,$value);
				$record[$key] = $select;
			}
		}	
	}






	// -------------------------------------------------------------------
	// Function name
	//   makeSelect
	// General description
	//   Creates a select box from fields and values
	// Inputs
	//   fields,values
	// Outputs
	//   select xml
	// Modified Date
	//   08/11/04
	//
	function makeSelectBox($fldName,$names,$values,$default,$submit = false,$onChange = ""){

		$selected = "";

		if($default != NULL && $default != ""){
			$selected = $default;
		}
		elseif(!empty($_REQUEST[$fldName])){
			$selected = $_REQUEST[$fldName];
		}


		if($submit){
			if($onChange != ""){
				$select = "<select name=\"$fldName\" onChange=\"$onChange\">\n";
			}
			else{
				$select = "<select name=\"$fldName\" onChange=\"javascript:this.form.submit();\">\n";
			}
		}
		else{
			$select = "<select name=\"$fldName\">\n";
		}

		for($i = 0;$i < count($names);$i++){
			$name = trim($names[$i]);
			$value = trim($values[$i]);

			if($selected == "" && $i == 0){
				$selected = $value;
			}

			if($selected != "" && $value == $selected){
				$select .= "\t<option value=\"$value\" selected>$name</option>\n";
			}
			else{
				$select .= "\t<option value=\"$value\">$name</option>\n";
			}
		}
		$select .= "</select>\n";

	return array($select,$selected);
	}

	// --------------------------------------------------------------
	function makeSimpleSelectBox($fldName,$names,$values,$default = NULL){

		$selected = "";
		if($default != NULL && $default != ""){
			$selected = $default;
		}
		elseif(!empty($_REQUEST[$fldName])){
			$selected = $_REQUEST[$fldName];
		}
		$select = "<select name=\"$fldName\">\n";
		for($i = 0;$i < count($names);$i++){
			$name = trim($names[$i]);
			$value = trim($values[$i]);

			if($selected == "" && $i == 0){
				$selected = $value;
			}

			if($selected != "" && $value == $selected){
				$select .= "\t<option value=\"$value\" selected>$name</option>\n";
			}
			else{
				$select .= "\t<option value=\"$value\">$name</option>\n";
			}
		}
		$select .= "</select>\n";
		return $select;
	}

	// --------------------------------------------------------------
	function makeSelectFromQuery($fldName,$nameFld,$valueFld,$sql,$default = null){
		
		global $_DB;
		
		$data = $_DB->getRecords($sql);
		
		if(count($data) == 0){
			return null;	
		}
		
		$selected = null;
		if(!empty($default) && !is_null($default)){
			$selected = $default;
		}
		elseif(!empty($_REQUEST[$fldName])){
			$selected = $_REQUEST[$fldName];
		}
		
		$select = "<select name=\"$fldName\">\n";
		$select .= "\t<option value=\"\"></option>\n";
		
		foreach($data as $i=>$fields){
			$value = $fields[$valueFld];
			$name = $fields[$nameFld]; 
			if(!is_null($selected) && $value == $selected){
				$select .= "\t<option value=\"$value\" selected>$name</option>\n";
			}
			else{
				$select .= "\t<option value=\"$value\">$name</option>\n";
			}
		}
		$select .= "</select>\n";
		return $select;
	}
	
	// --------------------------------------------------------------
	function printErrorMessage($errorTitle,$error){
		
		global $_isAdmin;
		$vars['error_title'] = $errorTitle;
		$vars['error_text'] = $error;
		// Load/display Template
		include_once("template.inc");
		$template = new Template();
		if($_isAdmin && file_exists("../templates/errors.html")){
			print $template->Open("../templates/errors.html",$vars);
		}
		elseif($_isAdmin && file_exists("../templates/error.messages.html")){
			print $template->Open("../templates/error.messages.html",$vars);
		}
		else{
			print $template->Open("templates/errors.html",$vars,true,true);
		}
		flush();
		exit;
	}

	// --------------------------------------------------------------
	function readCsvToArray($file){
		$csvData = array();
		$handle = fopen($file, "r");
		while (($data = fgetcsv($handle, 1000, ",")) !== FALSE) {
			$csvData[] = join("|",$data);
		}
		fclose($handle);
		return $csvData;
	}	


	// --------------------------------------------------------------
	function redirect($URL, $target = ""){
		if($target != ""){
			print "<meta http-equiv=\"refresh\" content=\"0;url=$URL\" target=\"$target\">\r\n";
		}
		else{
			print "<script language=\"JavaScript\">\n";
			print "window.location.replace('$URL');\n";
			print "</script>\n";
		}
	}


	// --------------------------------------------------------------
	function runSecurityCheck($page){

		$valid_page = false;

			// substitute spaces with %20

		$page = preg_replace("/ /","%20",$page);
		$page = preg_replace("/\+/","%20",$page);

			// get rid of attempts to insert illegal characters

		$page = preg_replace("/[\`\;\*\|\^\!\(\)\{\}\<\>\n\r]/","",$page);

			// Check for only "/" and word characters.
			// disallow "../"

		if(preg_match("/^[\w\/\?\:\%\&\=\._-]+$/",$page) && !preg_match("/\.{2}\//",$page)){
			$valid_page = true;
		}

		if(!$valid_page){
			print "<html><head><title>Access Denied!</title></head><body>\n";
			print "<font face=arial>\n";
			print "<p>Cannot Open: $page</P>";
			print "This is an illegal filename...</P>";
			print "</font></body></html>\n";
			exit;
		}
		return $page;
	}

	// --------------------------------------------------------------
	function sendMessage($message,$goback = false,$backNum = 1){
		print "<SCRIPT LANGUAGE=\"JavaScript\">\n";
		print "<!--\n";
		print "alert(\"" . $message . "\");\n";
		if($goback){
			print "history.go(-$backNum);\n";
		}
		print "-->\n";
		print "</SCRIPT>\n";
	}
	
	// --------------------------------------------------------------
	function unsetSessionVariable($var,$type = "session"){
		if($type == "session" && isset($_SESSION[$var])){
			$_SESSION[$var] = NULL;
			unset($_SESSION[$var]);
		}
		if($type == "request" && isset($_REQUEST[$var])){
			$_REQUEST[$var] = NULL;
			unset($_REQUEST[$var]);
		}
	}

	// --------------------------------------------------------------
	function optimizeTables($tName){
		global $_DB;
		// Check the status of the tables and if we have
		// "data free", optimize it.
		$srs = $_DB->execute("SHOW TABLE STATUS LIKE '$tName%'");
		while($status = $_DB->fetchRow($srs,"ASSOC")){
			if($status['data_free'] > 0){
				$table = $status['name'];
				$_DB->execute("OPTIMIZE TABLE $table");
			}
		}
	}



}

?>