<?php
// -----------------------------------------------------------------------
//         Class: Config
//       Purpose: This class controls the settings in the configuration.
// Last Modified: April 13, 2006
//
// -----------------------------------------------------------------------
class Config{

    var $debug = false;
    var $helpEditable = false;

    //-----------------------------------------------------
	function __construct(){

	}
	
	# -------------------------------------------------------------------
    function addtoSection(){
		global $_Template;

        // Get the sections for the list
        $sections = $this->getSections();

        $select = '<select name="section">\n';
        $select .= "  <option value=\"new_section\">Select Section</option>\n";
        foreach($sections as $index=>$name){
            $displayName = ucwords(strtolower(str_replace("_", " ", $name)));
            $select .= "  <option value=\"$name\">$displayName</option>\n";
        }
        $select .= "</select>\n";
		$vars['section_select'] = $select;

        // Load the template
		print $_Template->Open("templates/config.add.html",$vars);
        exit;
	}
	// --------------------------------------------------------------
	function addEntry(){

		global $_Common;
		global $_DB;
		global $refreshMenu;
		
		$section = NULL;
		if(!empty($_REQUEST['new_section'])){
			$section = strtolower(trim($_REQUEST['new_section']));
			$_REQUEST['section'] = $section;
		}
		elseif(!empty($_REQUEST['section']) && $_REQUEST['section'] != "new_section"){
			$section = strtolower($_REQUEST['section']);
		}
		else{
			$_Common->printErrorMessage("Configuration Error","The section name to add is missing.");	
		}
		
		$key = NULL;
		$value = NULL;
		$keyHelp = NULL;
		$sequence = 0;
		
		if(!empty($_REQUEST['key']) && !empty($_REQUEST['value'])){
			
			$key = strtolower(trim($_REQUEST['key']));
			$value = trim($_REQUEST['value']);
			
			$count = $_DB->getCount('config', "WHERE `section` = '$section' AND `key` = '$key'");
			if($count == 0){
				$cData = $_DB->getRecord("SELECT MAX(sequence) as seq FROM config WHERE `section` = '$section'");
				$sequence = $cData['seq'] + 1;
				$sql = "INSERT INTO config (`section`,`key`,`value`,`sequence`) VALUES ('$section','$key','$value','$sequence')";
				$_DB->execute($sql);
				$refreshMenu = true;
			}			
			if(!empty($_REQUEST['key_help'])){
				$count = $_DB->getCount('help', "WHERE `section` = '$section' AND `key` = '$key'");
				if($count == 0){
					$keyHelp = trim($_REQUEST['key_help']);				
					$sql = "INSERT INTO help (`section`,`key`,`key_help`) VALUES ('$section','$key','$keyHelp')";
					$_DB->execute($sql);
				}
			}
		}
	}	
	//-----------------------------------------------------
	function read($sectionToFind = NULL,$startup = false){

		global $_CF;
		global $_Common;
		global $_DB;
		global $_isAdmin;
		
		$sql = "SELECT * FROM config ORDER BY section, sequence";
		
        $rs = $_DB->execute($sql);
        $rows = array();

		while($row = $_DB->fetchrow($rs, "ASSOC")){
			
			//$_Common->debugPrint($row);
			
			$section = trim($row['section']);
			$key = trim($row['key']);
			$value = stripslashes(trim($row['value']));
			
			if(!$_isAdmin && (strtolower($value) == "true" || strtolower($value) == "yes")){
				$value = true;	
			}
			if(!$_isAdmin && (strtolower($value) == "false" || strtolower($value) == "no")){
				$value = false;	
			}
		
			$_CF[$section][$key] = $value;
		}


		// If startup, load general shipping and sales table vars into cf
		if($startup){
			$sql = "SELECT * FROM shipping";
			$shipData = $_DB->getRecord($sql);
			foreach($shipData as $key=>$value){
				
				if((strtolower($value) == "true" || strtolower($value) == "yes")){
					$value = true;	
				}
				if((strtolower($value) == "false" || strtolower($value) == "no")){
					$value = false;	
				}
				
				if($key != "shid"){
					$_CF['shipping'][$key] = $value;
				}
			}

			$sql = "SELECT * FROM sales_tax";
			$taxData = $_DB->getRecord($sql);
			foreach($taxData as $key=>$value){
				
				if((strtolower($value) == "true" || strtolower($value) == "yes")){
					$value = true;	
				}
				if((strtolower($value) == "false" || strtolower($value) == "no")){
					$value = false;	
				}
				
				if($key != "stid" || $key != "section_help"){
					$_CF['sales_tax'][$key] = $value;
				}
			}
		}

		// allows for custom CF settings when the same database is being used
		// for more than one store on the server.
		if(file_exists("include/store.config.php")){
			$custom = parse_ini_file("include/store.config.php",true);
			foreach($custom as $section=>$flds){
				foreach($flds as $cKey=>$cVal){
					$_CF[strtolower(trim($section))][strtolower(trim($cKey))] = trim($cVal);
				}
			}
		}

		if($sectionToFind && isset($_CF[$sectionToFind])){
			return $_CF[$sectionToFind];
		}
	}
	
	// --------------------------------------------------------------
    function getSection(){
		
		global $_Common;
		global $_DB;
		global $refreshMenu;
		global $_Template;
		
		$vars = array();
		$section = NULL;
		if(!empty($_REQUEST['section'])){
			$section = trim($_REQUEST['section']);
			$vars['section'] = trim($_REQUEST['section']);
			$vars['section_label'] = ucwords(strtolower(str_replace("_", " ",$section)));
		}
		else{
			$_Common->printErrorMessage("Missing Section","Missing Section to edit?");
			exit;
		}
		
		$vars['section_help'] = "";

		$sql = "SELECT section_help FROM help WHERE section_help != '' AND section = '$section'";
		$help = $_DB->getRecord($sql);
		if(!empty($help['section_help'])){
			$vars['section_help'] = $help['section_help'];
		}
		
		$sql = "SELECT config.*,help.key_help FROM config
				LEFT JOIN help ON config.key = help.key
				WHERE config.section = '$section'
				AND (help.section = '$section' OR help.section IS NULL)
				ORDER BY config.sequence";

		$vars['settings'] = array();
				
		$rs = $_DB->execute($sql);
		while($row = $_DB->fetchrow($rs, "ASSOC")){
			//$_Common->debugPrint($row);
			$vars['settings'][] = $_Common->array_trim($row);
		}
		
		//$_Common->debugPrint($vars['settings']);
		//exit;
		
		$_DB->free_result($rs);

		$vars['refreshMenu'] = $refreshMenu;
		$vars['helpEditable'] = $this->helpEditable;
		print $_Template->Open("templates/config.edit.html",$vars);
		exit;
	}
	
	//-----------------------------------------------------
	function add($section,$key,$value,$sequence = NULL){
		global $_DB;
		global $_isAdmin;
		if($_isAdmin){
			if(!$sequence){
				$sequence = 0;	
			}
			$sql = "INSERT INTO config (`section`,`sequence`,`key`,`value`) VALUES('$section','$sequence','$key','$value')";
			$rs = $_DB->execute($sql);
			$_DB->free_result($rs);
		}
	}
	//-----------------------------------------------------
	function update($section,$key,$value,$sequence = NULL){
		global $_DB;
		global $_isAdmin;
		if($_isAdmin){
			if(!$sequence){
				$sequence = 0;	
			}
			$sql = "UPDATE config SET `section` = '$section',`sequence` = '$sequence',`key` = '$key',`value` = '$value'";
			$rs = $_DB->execute($sql);
			$_DB->free_result($rs);
		}
	}

	// --------------------------------------------------------------
	function updateSection(){
		global $_Common;
		global $_DB;
		$section = NULL;
		if(!empty($_REQUEST['section'])){
			$section = strtolower($_REQUEST['section']);
		}
		else{
			$_Common->printErrorMessage("Configuration Error","The section name to edit is missing.");	
		}

		// If we are in develop mode, save help entries
		if($this->helpEditable && isset($_REQUEST['help']) && is_array($_REQUEST['help'])){
			if(isset($_REQUEST['section_help'])){
				$sHelp = trim($_REQUEST['section_help']);
				$_DB->escape_string($sHelp);
				// check to see if single section help entry exists
				$sql = "SELECT * FROM help WHERE section_help != '' AND `section` = '$section' AND `key` = ''";
				$hData = $_DB->fetchrow($_DB->execute($sql), "ASSOC");
				if(isset($hData['hid'])){
					$hid = $hData['hid'];
					$sql = "UPDATE help SET `section_help` = '$sHelp' WHERE `section` = '$section' AND hid = '$hid'";
				}
				else{
					$sql = "INSERT INTO help (`section`,`section_help`) VALUES('$section','$sHelp')";
				}
				$_DB->execute($sql);
			}
			foreach($_REQUEST['help'] as $hKey=>$hValue){
				$hValue = stripslashes(trim($hValue));
				$_DB->escape_string($hValue);
				// check to see if single section help entry exists
				$sql = "SELECT * FROM help WHERE key_help != '' AND `section` = '$section' AND `key` = '$hKey'";
				$hData = $_DB->fetchrow($_DB->execute($sql), "ASSOC");
				if(isset($hData['hid'])){
					$hid = $hData['hid'];
					$sql = "UPDATE help SET `key_help` = '$hValue' WHERE `section` = '$section' AND `key` = '$hKey' AND hid = '$hid'";
				}
				else{
					$sql = "INSERT INTO help (`section`,`key`,`key_help`) VALUES('$section','$hKey','$hValue')";
				}
				$_DB->execute($sql);
			}
			foreach($_REQUEST['sequence'] as $sKey=>$sValue){
				$sql = "UPDATE config SET `sequence` = '$sValue' WHERE `section` = '$section' AND `key` = '$sKey'";
				$_DB->execute($sql);
			}
		}

		if(isset($_REQUEST['accepted_payment_methods']) && is_array($_REQUEST['accepted_payment_methods'])){
			$_DB->execute("TRUNCATE TABLE payment_methods");
			foreach($_REQUEST['accepted_payment_methods'] as $k=>$method){
				$sql = "INSERT INTO payment_methods (`method`,`active`) VALUES('$method','true')";
				$_DB->execute($sql);
			}
		}
		if(isset($_REQUEST['credit_card_types']) && is_array($_REQUEST['credit_card_types'])){
			$_DB->execute("UPDATE credit_cards SET `active` = 'false'");
			foreach($_REQUEST['credit_card_types'] as $cname=>$active){
				$sql = "UPDATE credit_cards SET `active` = 'true' WHERE card_name = '$cname'";
				$_DB->execute($sql);
			}
			// add any new cards
			if(isset($_REQUEST['new_credit_card']) && trim($_REQUEST['new_credit_card']) != ""){
				$newCard = trim($_REQUEST['new_credit_card']);
				$_DB->execute("INSERT INTO credit_cards (`card_name`,`active`) VALUES('$newCard','true')");
			}
		}

		// Update config key values
		foreach($_REQUEST as $key=>$value){
			if(!is_array($value)){
				$_DB->escape_string($value);
				$sql = "UPDATE config SET `value` = '$value' WHERE `section` = '$section' AND `key` = '$key'";
				$_DB->execute($sql);
			}
		}
	
	}
	
	// ----------------------------------------------------------------
	function getSections(){
		global $_DB;
		$sql = "SELECT DISTINCT section FROM config ORDER BY section";
		$sections = array_keys($_DB->getRecords($sql,'section'));
		
		
		
		return $sections;
	}

	// ----------------------------------------------------------------
	function displayMenu(){
		
		global $_Common;
		global $_DB;
		global $_Template;
		$vars['sections'] = $this->getSections();
		
		// remove extensions. This is edited seperatly
		$found = array_search('extensions',$vars['sections']);
		if($found){
			unset($vars['sections'][$found]);	
		}
		print $_Template->Open("menus/config.menu.html",$vars);
		exit;
	}

	// --------------------------------------------------------------
	function readINIfile($filename, $commentchar = ';') {
		
		$array1 = file($filename);
		$section = '';
		
		foreach ($array1 as $i=>$filedata) {
			$dataline = trim($filedata);
			$firstchar = substr($dataline, 0, 1);
			if($firstchar != $commentchar && $dataline != '') {
				//It's an entry (not a comment and not a blank line)
				if ($firstchar == '[' && substr($dataline, -1, 1) == ']') {
					//It's a section
					$section = strtolower(substr($dataline, 1, -1));
				}
				else{
					//It's a key...
					$delimiter = strpos($dataline, '=');
					if ($delimiter > 0) {
						//...with a value
						$key = trim(substr($dataline, 0, $delimiter));
						$value = trim(substr($dataline, $delimiter + 1));
						if(substr($value, 1, 1) == '"' && substr($value, -1, 1) == '"') {
							$value = substr($value, 1, -1); 
						}
						//$key = strtolower($key);
						
						if(strtolower($value) == "true" || strtolower($value) == "yes"){
							$value = true;	
						}
						elseif(strtolower($value) == "false" || strtolower($value) == "no"){
							$value = false;	
						}
						else{
							$value = str_replace('"','',$value);
							
							$value = stripcslashes($value);
						}
						
						$array2[$section][$key] = $value;
					}
					else{
						//...without a value
						$array2[$section][trim($dataline)]='';
					}
				}
			}
		}
		return $array2;
	}
	
}

?>