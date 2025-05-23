<?php
//VersionInfo:Version[3.0.1]

class Options{

    var $debug = false;
    
    // -------------------------------------------------------------------
    function __construct(){
	}
	
	// -------------------------------------------------------------------
    function listOptions(){
		
		global $_Common;
		global $data;
		global $_DB;
		global $selectedOptions;

		$pname = NULL;
		if(!empty($_REQUEST['pname'])){
			$pname = trim($_REQUEST['pname']);
		}

		// See if we have a pid, then we are selecting options

        $pid = NULL;
		if(!empty($_REQUEST['pid'])){
			$pid = trim($_REQUEST['pid']);
		}
        
        $selectedOptions = array();
        
        if($pid){
			// get list for selected values
            $sql = "SELECT pid,oid,sequence FROM product_options WHERE pid = '$pid'";
            $pidOptions = $_DB->getRecords($sql);
            foreach($pidOptions as $index=>$flds){
                $selectedOptions[$flds['oid']] = $flds;
            }
        }
            // Get a list of all the option_save_names

        $sql = "SELECT oid,name,description,required FROM options ";

        $data = $_DB->getRecords($sql);

		if($this->debug){
			$_Common->debugprint($data);
		}
	}
	
	// -------------------------------------------------------------------
	function applyOptions(){
		
		global $_Common;
		global $_DB;
		global $_Registry;

		$pname = NULL;
		if(!empty($_REQUEST['pname'])){
			$pname = trim($_REQUEST['pname']);
		}

		// See if we have a pid, then we are selecting options

        $pid = NULL;
		if(!empty($_REQUEST['pid'])){
			$pid = trim($_REQUEST['pid']);
		}


		if($pid){

			$optionsAreInventoried = false;
			if($_Registry->file_exists_incpath("inventory.inc")){
				$invOptionCount = $_DB->getCount('products',"WHERE pid = '$pid' AND inventory_item = 'true' AND inventory_options = 'true'");
				if($invOptionCount > 0){
					$optionsAreInventoried = true;
					$inventory = $_Registry->loadClass('Inventory');
				}
			}

			//Remove any existing options
			$sql = "DELETE FROM product_options WHERE pid = '$pid'";
			//print "<pre>$sql\n";
			$result = $_DB->execute($sql);

			$_DB->execute("OPTIMIZE TABLE `product_options`");

			// Link up the new ones
			$oids = array();
			if(!empty($_REQUEST['oid'])){
				$oids = $_REQUEST['oid'];
			}
//
//$_Common->debugprint($oids);
//$_Common->debugprint($_REQUEST['sequence']);
//exit;
			
			if(count($oids) > 0){
				$count = 0;
				foreach($oids as $index=>$oid){
					$count++;
					$sequence = "";
					if(isset($_REQUEST['sequence'][$oid]) && $_REQUEST['sequence'][$oid] != ""){
						$sequence = $_REQUEST['sequence'][$oid];	
					}
					else{
						$sequence = $count;	
					}
					$sql = "INSERT INTO product_options (pid,oid,sequence) VALUES ('$pid','$oid','$sequence')";
					//print "<pre>$sql\n";
					$result = $_DB->execute($sql);
					
					$message = "$count options were applied to: $pname";
					
					if($optionsAreInventoried){
						// reset the inventory options
						if($_DB->getCount('inventory', "WHERE pid = '$pid'") > 0){
							$inventory->RemoveOptionsFromInventory($pid);
							$message .= "\n\nThe option inventory has also been reset because you modified the options.";
						}
					}
				}
			}
			else{
				$message = "All options for the $pname product have been deleted.";
			}
			$_Common->sendMessage($message);
		}
	}
	

	// -------------------------------------------------------------------
    function editOptions(){
		
		global $_Common;
		global $data;
		global $_DB;
		global $isInventoried;
		global $options;
		
		$oid = NULL;
		if(!empty($_REQUEST['oid'])){
			$oid = $_REQUEST['oid'];
		}

		// Get default values
		$sql = "SELECT * FROM product_edit WHERE related_table = 'options'";
		$rs = $_DB->execute($sql);
		while($row = $_DB->fetchrow($rs, "ASSOC")){
			$data[$row['key']] = $row['value'];
		}
		$_DB->free_result($rs);

		if($oid){

			$sql = "SELECT options.*,option_details.* FROM options,option_details WHERE options.oid = '$oid'
		            AND options.oid = option_details.oid ORDER BY option_details.sequence,option_details.odid";
		            
			$count = 0;
			$rs = $_DB->execute($sql);
			$count = $_DB->numrows($rs);
			$data['option_rows'] = $count;
			$odids = array();
			if($count > 0){
				while($row = $_DB->fetchrow($rs, "ASSOC")){
					
					//$_Common->debugPrint($row);
					
					$data['oid'] = $row['oid'];
					$data['name'] = $row['name'];
					$data['format'] = $row['format'];
					$data['required'] = $row['required'];
					$data['type'] = $row['type'];
					$options[] = $row;
					$odids[$row['odid']] = $row['odid'];
				}
			}
			$_DB->free_result($rs);

			foreach($odids as $odid=>$junk){
				// check inventory for options
				$count = $_DB->getCount('inventory', "WHERE LOCATE('$odid',odids)");
				if($count > 0){
					$isInventoried = true;
					break;
				}
			}


			
			//$_Common->debugPrint($data);
			//$_Common->debugPrint($options);
			
		}

	}

	// -------------------------------------------------------------------
    function updateOptions(){
		
		global $_Common;
		global $_DB;
		global $fldProperties;
		global $_Registry;
		$inventory = NULL;
		
		// get inventory routines
		if($_Registry->file_exists_incpath("inventory.inc")){
			$inventory = $_Registry->LoadClass("inventory");
		}
		
        // Use the values to create the description
        // limit the description text to 100 chars

		if(!empty($_REQUEST['value'])){
			$oValues = $_REQUEST['value'];
			$oDesc = "";
			foreach($oValues as $index=>$oVal){
				if(trim($oVal) == ""){
					continue;
				}
				if($oDesc == ""){
					$oDesc .= trim($oVal);
				}
				else{
					$oDesc .= "," . trim($oVal);
				}
			}
			$oDesc = substr($oDesc,0,100);
			if(strlen($oDesc) == 100){
				$oDesc .= "...";
			}
			$_REQUEST['description'] = $oDesc;
		}

		// We don't use this option_number in 3.0 but it comes in from the
		// 2.12 import so just set to 1.

		$_REQUEST['number'] = "1";

		// Update default values
		$sql = "SELECT * FROM product_edit WHERE related_table = 'options'";
		$rs = $_DB->execute($sql);
		while($row = $_DB->fetchrow($rs, "ASSOC")){
			if(!empty($_REQUEST[$row['key']]) && trim($_REQUEST[$row['key']]) != trim($row['value'])){
				$oldKey = $row['key'];
				$newVal = trim($_REQUEST[$row['key']]);
				$sql = "UPDATE product_edit SET `value` = '$newVal' WHERE `key` = '$oldKey' AND related_table = 'options'";
				if($this->debug){
					$_Common->debugPrint($sql);
				}
				$_DB->execute($sql);
			}
		}
		$_DB->free_result($rs);

		$oid = NULL;
		$odids = $_REQUEST['odid'];
		$sequence = $_REQUEST['sequence'];				
		$values = $_REQUEST['value'];
		$prices = $_REQUEST['price'];
		$weights = $_REQUEST['weight'];
		$texts = $_REQUEST['text'];
		
		if(isset($_REQUEST['insert'])){

            // Insert a new option.
			list($fields, $vals) = $_DB->makeAddFields($fldProperties, 'oid', $_REQUEST);
			
			$sql = "INSERT INTO options ($fields) VALUES ($vals)";
            $result = $_DB->execute($sql);

            // Get the new "oid";
            $oid = $_DB->getInsertID("options","oid");

			// Now put in the new option values
			if(count($values) > 0){
				$oOrder = 1;
				for($i=0;$i<count($values);$i++){

					$value = "";
					$price = "";
					$weight = "";
					$text = "";
					$order = '';

					if(trim($values[$i]) != ""){
						$value = trim($values[$i]);
		                
						$price = trim($prices[$i]);
						$price = ($price==""?0:$price);
		                
						$weight = trim($weights[$i]);
						$weight = ($weight==""?0:$weight);
		                
						$text = trim($texts[$i]);
						$text = ($text=""?$value:$text);
		                
						$order = trim($sequence[$i]);
						$order = ($order=='0'?$oOrder:$order);

						$sql = "INSERT INTO option_details (`oid`,`sequence`,`value`,`price`,`weight`,`text`)
													VALUES ('$oid','$order','$value','$price','$weight','$text')";
						if($this->debug){
							$_Common->debugPrint($sql);
						}	
						$rs = $_DB->execute($sql);
						$_DB->free_result($rs);
					}
					$oOrder++;
				}
			}
		}
		else{
		
			if(!empty($_REQUEST['oid'])){

				// Update options table
				$oid = $_REQUEST['oid'];
				$updateFlds = $_DB->makeUpdateFields($fldProperties,'oid',$_REQUEST);
				$sql = "UPDATE options SET $updateFlds WHERE oid = '$oid'";
				if($this->debug){
					$_Common->debugPrint($sql);
				}	
				$rs = $_DB->execute($sql);
				$_DB->free_result($rs);
				
                // Delete the existing option details so we make sure
                // we have all the data from the new entries correct.
//				$sql = "DELETE FROM option_details WHERE oid = '$oid'";
//				if($this->debug){
//					$_Common->debugPrint($sql);
//				}	
//				$rs = $_DB->execute($sql);
//				$_DB->free_result($rs);
//				$_DB->execute("OPTIMIZE TABLE `option_details`");

				$sql = "SELECT odid,value FROM option_details WHERE oid = '$oid'";
				$details = $_DB->getRecords($sql,'odid');

				//$_Common->debugPrint($_REQUEST);
				//$_Common->debugPrint($details);

				$resetInventory = false;
				$oOrder = 1;
				for($i=0;$i<count($values);$i++){

					$value = "";
					$price = "";
					$weight = "";
					$text = "";
					$order = "";

					if(trim($values[$i]) != ""){
						$value = trim($values[$i]);
						$price = trim($prices[$i]);
						$price = ($price==""?0:$price);
						$weight = trim($weights[$i]);
						$weight = ($weight==""?0:$weight);
						$text = trim($texts[$i]);
						$text = ($text=""?$value:$text);
						$order = trim($sequence[$i]);
						$order = ($order=='0'?$oOrder:$order);
					}
					else{
						continue;	
					}

					
					if(!empty($odids[$i])){
						
						$odid = $odids[$i];
						
						if(isset($details[$odid]) && $value != ""){
							// If the value has changed, update it in the inventory table.
							if(isset($details[$odid]['value']) && trim($details[$odid]['value']) != $value){
								if($inventory){
									$inventory->updateInventoryOption($odid,$details[$odid]['value'],$value);
								}
							}
							$sql = "UPDATE option_details SET `sequence` = '$order',`value` = '$value',`price` = '$price',`weight` = '$weight',`text` = '$text' WHERE odid = '$odid'";
							// remove from list so we can check it later
							unset($details[$odid]);
						}
						else{
							if($value != ""){
								$sql = "INSERT INTO option_details (`oid`,`sequence`,`value`,`price`,`weight`,`text`)
															VALUES ('$oid','$order','$value','$price','$weight','$text')";
								// we have a new option detail, need to reset the inventory
								$resetInventory = true;
							}
						}
					}
					else{
						if($value != ""){
							$sql = "INSERT INTO option_details (`oid`,`sequence`,`value`,`price`,`weight`,`text`)
														VALUES ('$oid','$order','$value','$price','$weight','$text')";
							$resetInventory = true;
						}
					}
					
					if($this->debug){
						$_Common->debugPrint($sql);
					}	
					$rs = $_DB->execute($sql);
					$_DB->free_result($rs);
					
					$oOrder++;
				}

				// they removed some options and did not update those lines
				if(count($details) > 0){
					$resetInventory = true;
					foreach($details as $k=>$fields){
						$odid = trim($fields['odid']);
						if($odid != ""){
							$sql = "DELETE FROM option_details WHERE odid = '$odid'";
							if($this->debug){
								$_Common->debugPrint($sql,"remove option");
							}
							$rs = $_DB->execute($sql);
							$_DB->free_result($rs);
						}
					}
				}

				// Options were added or removed so update inventory if we have it				
				if($inventory && $resetInventory){
					$sql = "DELETE FROM inventory WHERE LOCATE('$oid',oids)";
					if($this->debug){
						$_Common->debugPrint($sql,"reset inventory");
					}
					$_DB->execute($sql);
					$_DB->execute("OPTIMIZE TABLE `inventory`");
					$_Common->sendMessage("The inventory records for this option have been reset.");	
				}
			}
		}
		
		//exit;
		
	}

	// -------------------------------------------------------------------
    function deleteOptions(){
		global $_Common;
		global $_DB;
		$oid = NULL;
		if(!empty($_REQUEST['oid'])){
			$oid = $_REQUEST['oid'];
			$_DB->execute("DELETE FROM options WHERE oid = '$oid'");
			$_DB->execute("DELETE FROM option_details WHERE oid = '$oid'");
			$_DB->execute("DELETE FROM product_options WHERE oid = '$oid'");
			$_DB->execute("DELETE FROM inventory WHERE LOCATE('$oid',oids)");
		}
	}
    
    
}

?>