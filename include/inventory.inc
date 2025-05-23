<?php
//VersionInfo:Version[3.0.1]

class Inventory {

    var $debug = false;
    var $soldOutMessage = "The %name% is currently sold out.";
    var $qtyNotAvailableMessage = "The quantity you selected for the %name% is higher than the available quantity (%count%)";
    var $qtyInCartMessage = "\\n\\nYou currently have %quantity% of these in your cart.";
    var $qtyInCartReset = "\\n\\nThe quantity was automatically reset to %quantity% for this item in your cart.";
    
    // --------------------------------------------------------------
    function __construct(){
		global $_DB;
		if(!$_DB->mysql_table_exists("inventory")){
			$this->makeInventoryTable();
		}
	}
	
	// --------------------------------------------------------------
	// Gets a list of the options for the control panel
    function GetInventory($pid,$showOptions){

        global $_DB;
        global $_Common;
        $haveOptions = false;
        
		$sql = "SELECT * FROM inventory WHERE pid = '$pid'";
		$inventoryData = $_DB->getRecords($sql);

		if($this->debug){
			$_Common->debugPrint($inventoryData,"Loading inventory data: $sql");
		}

		// If we turned on the show options, and we have basic inventory data,
		// delete it and reset it to the add the list of options.
		if($showOptions && empty($inventoryData[0]['odids'])){
			if(count($inventoryData) > 0){
				$this->DeleteInventory($pid);	
			}
			$invDetails = array();
			$this->loadInventoriedOptions($invDetails,$pid);
			$inventoryData = $invDetails;
		}

		if($this->debug){
			$_Common->debugPrint($inventoryData,"loaded inventory data");
		}
		
		return $inventoryData; 
    }


	// --------------------------------------------------------------
	// This loads the starting inventory list
	// --------------------------------------------------------------
	function loadInventoriedOptions(&$invDetails,$pid){
		
		global $_Common;
		global $_DB;
		
		// Get a list of the oid's in the product_options so we know how many options to merge
		$sql = "SELECT oid FROM product_options WHERE pid = '$pid'";
		$pOptions = $_DB->getRecords($sql,'oid');

		if(count($pOptions) > 0){
		
			$optionDetails = array();
			foreach(array_keys($pOptions) as $i=>$oid){
				$sql = "SELECT * FROM option_details WHERE oid = '$oid'";
				$optionDetails[] = $_DB->getRecords($sql);
			}
			
			// init details with first set of options
			$invDetails = array();
			$optGroupCount = count($optionDetails);
			
			foreach($optionDetails[0] as $key=>$odRow){
				
				if($optGroupCount == 1){
					$invDetails[] = array('pid' => $pid,
										'oids' => $odRow['oid'],
										'odids' => $odRow['odid'],
										'value' => $odRow['value'],
										'name' => $odRow['value'],
										'minimum_quantity' => 0,
										'maximum_quantity' => 0,
										'quantity_available' => 0,
										'quantity_sold' => 0);
				}
				elseif($optGroupCount > 1){
					$invDetails[] = array('pid' => $pid,
										'oid' => $odRow['oid'],
										'odid' => $odRow['odid'],
										'value' => $odRow['value'],
										'name' => $odRow['value'],
										'minimum_quantity' => 0,
										'maximum_quantity' => 0,
										'quantity_available' => 0,
										'quantity_sold' => 0);
				}
			}
			if($optGroupCount > 1){
				// multiply out the options.
				for($i = 1; $i<$optGroupCount;$i++){
					$optGroup = $optionDetails[$i];
					$oldInvDetails = $invDetails;
					$invDetails = array();
					foreach($optGroup as $key=>$odRow){
						foreach($oldInvDetails as $key2=>$value){
							$row['pid'] = $value['pid'];
							$row['oids'] = $value['oid'] . ":" . $odRow['oid'];
							$row['odids'] = $value['odid'] . ":" . $odRow['odid'];
							$row['name'] = $value['value'] . ":" . $odRow['value'];
							$row['minimum_quantity'] = $value['minimum_quantity'];
							$row['maximum_quantity'] = $value['maximum_quantity'];
							$row['quantity_available'] = $value['quantity_available'];
							$row['quantity_sold'] = $value['quantity_sold'];
							$invDetails[] = $row;
						}
					}
				}
			}
		}	
	}

	// -------------------------------------------------------------------
	// gets the inventory count for the cart
	function checkInventoryForCart($sku,$qty,$options = array()){
		
		global $_Common;
		global $_DB;
		
		$optionNames = array();
		$oNames = null;
		if(count($options) > 0){
			foreach($options as $i=>$flds){
				$optionNames[] = $flds['value'];
			}
			$oNames = join(':',$optionNames);
		}
		$sql = "SELECT products.pid,products.inventory_item,products.display_when_sold_out,inventory.quantity_available
				FROM products,inventory
				WHERE inventory.pid = products.pid
				AND products.sku = '$sku'";
		if($oNames){
			$sql .= " AND inventory.name = '$oNames'";
		}
		$inventoryData = $_DB->getRecord($sql);
		if(count($optionNames) > 0){
			$inventoryData['options'] = join(', ',$optionNames);
		}
		else{
			$inventoryData['options'] = null;
		}

		if($this->debug){
			$_Common->debugPrint($inventoryData,"Inventory Data: $sql");	
		}

		return $inventoryData;	
	}


	// -------------------------------------------------------------------
	// Just updates the option names in the inventory table
    function updateInventoryOption($odid,$oldValue,$newValue){
		
		global $_Common;
		global $_DB;

		$sql = "SELECT name,odids FROM inventory WHERE LOCATE('$odid',odids) AND LOCATE('$oldValue',name)";
		$records = $_DB->getRecords($sql);
		
		foreach($records as $i=>$flds){
			$modify = false;
			$odidsValue = $flds['odids'];
			$odids = explode(":",$odidsValue);
			$names = explode(":",$flds['name']);
			foreach($odids as $j=>$id){
				if($id == $odid && $names[$j] == $oldValue){
					$names[$j] = $newValue;
					$modify = true;
				}
			}	
			if($modify){
				$newName = join(":",$names);
				$sql = "UPDATE inventory SET `name` = '$newName' WHERE odids = '$odidsValue' LIMIT 1";
				if($this->debug){
					$_Common->debugPrint($sql);
				}
				$rs = $_DB->execute($sql);
				$_DB->free_result($rs);
			}
		}
	}

    
	// --------------------------------------------------------------
	// Updates the inventory records from the admin panel
    function SetInventory($pid){
		
		global $_Common;
		global $_DB;

		//$this->debug = true;
		
		if(!empty($_REQUEST['inventory']) && count($_REQUEST['inventory']) > 0){
			
			if($this->debug){
				$_Common->debugPrint($_REQUEST['inventory'],"Inventory Counts");	
			}
			
			$fieldProperties = $_DB->getFieldProperties('inventory');
			
			foreach($_REQUEST['inventory'] as $i=>$flds){
				
				if(!isset($flds['minimum_quantity']) || $flds['minimum_quantity'] == ""){
					$flds['minimum_quantity'] = 1;
				} 
				if(!isset($flds['quantity_available']) || $flds['quantity_available'] == ""){
					$flds['quantity_available'] = 1;
				} 
				if(!isset($flds['quantity_sold']) || $flds['quantity_sold'] == ""){
					$flds['quantity_sold'] = 0;
				} 
				$name = $flds['name'];
				
				if(!empty($flds['odids'])){
					$odids = $flds['odids'];
					$count = $_DB->getCount('inventory', "WHERE pid = '$pid' AND odids = '$odids'");
					if($count > 0){
						$values = $_DB->makeUpdateFields($fieldProperties,'iid',$flds);
						$sql = "UPDATE inventory SET $values WHERE pid = '$pid' AND odids = '$odids' LIMIT 1";
					}
					else{
						list($fields,$values) = $_DB->makeAddFields($fieldProperties,'iid',$flds);
						$sql = "INSERT INTO inventory ($fields) VALUES($values)";
					}
					if($this->debug){
						$_Common->debugPrint($sql);	
					}
					$_DB->execute($sql);
				}
				else{
					$count = $_DB->getCount('inventory', "WHERE `pid` = '$pid' AND `name` = '$name'");
					if($count > 0){
						$values = $_DB->makeUpdateFields($fieldProperties,'iid',$flds);
						$sql = "UPDATE inventory SET $values WHERE pid = '$pid' AND `name` = '$name' LIMIT 1";
					}
					else{
						list($fields,$values) = $_DB->makeAddFields($fieldProperties,'iid',$flds);
						$sql = "INSERT INTO inventory ($fields) VALUES($values)";
					}
					if($this->debug){
						$_Common->debugPrint($sql);	
					}
					$_DB->execute($sql);
				}
			}
		}
		//exit;
	}

	// --------------------------------------------------------------
	// Update inventory at checkout. Use the merged names of the options instead
	// of the odids because we don't have them in the cart.
    function UpdateInventory(){
		
		global $_CART;
		global $_Common;
		global $_DB;

		$soldOutItems = array();

		foreach($_CART as $i=>$flds){
			
			if(isset($flds['pid'])){
				
				$qty = $flds['quantity'];
				$pid = $flds['pid'];
				$name = $flds['name'];
				
				$where = "WHERE inventory.pid = '$pid'";
				
				$optionNames = array();
				$oNames = null;
				if(isset($flds['options']) && count($flds['options']) > 0){
					foreach($flds['options'] as $i=>$oflds){
						$optionNames[] = $oflds['value'];
					}
					$oNames = strtolower(join(':',$optionNames));
					$name .= ':' . $oNames; 
				}

				$AND = " AND inventory.pid = '$pid'";
				//	Marcello do not create option query for clearance items		
				//if($oNames){
				if(($oNames) && (stristr($oNames,'clearance') === FALSE)){
					$AND .= " AND LOWER(inventory.name) = '$oNames'";
					$where .= " AND LOWER(inventory.name) = '$oNames'";
				}
				
				//$_Common->debugPrint("Option names marcello ",$oNames); //marcello
				
				$available = 0;
				
				$sql = "SELECT products.inventory_item,inventory.quantity_available
						FROM products, inventory
						WHERE products.pid = inventory.pid $AND";
				
				$testdata = $_DB->getRecords($sql);
										 
				//$_Common->debugPrint($testdata,$sql);
				
				// Some of the entries went in without the name in the field
				// and it caused duplicate entries. lets do some cleanup here.
				if(count($testdata) > 1){
					$iid = null;
					$haveName = false;
					foreach($testdata as $j=>$fields){
						if(!empty($fields['name'])){
							if(!empty($fields['iid'])){
								$iid = $fields['iid'];
							}
						}
						else{
							$haveName = true;
						}	
					}
					// make sure we have at least one entry with the name
					if($haveName && $iid){
						$_DB->execute("DELETE FROM inventory WHERE iid = '$iid'");
					}
					elseif(!$haveName && $iid){
						$_DB->execute("UPDATE inventory SET `name` = '$name' WHERE `pid` = '$pid'");
					}
				}
				
				$data = $_DB->getRecord($sql);
				
				if(count($data) > 0){
					
					if(!empty($data['inventory_item']) && $data['inventory_item'] == 'true'){
					
						$available = ($data['quantity_available'] - $qty);
						if($available < 0){
							$available = 0;
						}
						if($available == 0){
							$soldOutItems[$pid] = $name;
						}
						$sql = "UPDATE inventory SET quantity_available = '$available',
													 quantity_sold = (quantity_sold + $qty) $where LIMIT 1";

						if($this->debug){
							$_Common->debugPrint($sql,"Update Inventory Counts");	
						}
						$_DB->execute($sql);
					}
				}
			}
		}
		
		if(count($soldOutItems) > 0){
			$this->notifyAdmin($soldOutItems);	
		}
		
		
	}

	// --------------------------------------------------------------
	function notifyAdmin($soldOutItems){
		
		global $_CF, $_Registry;
		
		$from = $_CF['email']['store_email_address'];
		$to = $_CF['email']['store_email_address'];
		$subject = "Sold Out Inventory Items";
		$body = "\r\n\r\n";
		$body .= "The following items have been sold out:\r\n\r\n";
		foreach($soldOutItems as $pid=>$name){
			$body .= "$pid - $name\r\n";
		}
		$html = "<html><head><title>$subject</title></head><body><pre>$body</pre></body></html>";
		$email = $_Registry->loadClass('email');
		$email->send($from, $to, $subject, $html, $body);
	}


	// --------------------------------------------------------------
	// If we had options in the inventory and it gets turned off in
	// the control panel, remove the option records from inventory.
    function RemoveOptionsFromInventory($pid){
		global $_DB;
		if(trim($pid) != "" && $pid != "0"){
			$sql = "DELETE FROM inventory WHERE pid = '$pid' AND odids != ''";
			$_DB->execute($sql);
			$_DB->execute("OPTIMIZE TABLE `inventory`");
		}
	}

	// --------------------------------------------------------------
	// Removes all records from inventory
    function DeleteInventory($pid){
		global $_DB;
		if(trim($pid) != "" && $pid != "0"){
			$sql = "DELETE FROM inventory WHERE pid = '$pid'";
			$_DB->execute($sql);
			$_DB->execute("OPTIMIZE TABLE `inventory`");
		}
	}
	
	// --------------------------------------------------------------
	// creates the inventory table if not exists
	function makeInventoryTable(){
		global $_DB;
		$sql = "CREATE TABLE IF NOT EXISTS `inventory` (
				`iid` int(10) NOT NULL default '0',
				`pid` int(10) NOT NULL default '0',
				`odids` varchar(50) NOT NULL default '',
				`name` varchar(255) NOT NULL default '',
				`minimum_quantity` int(10) NOT NULL default '0',
				`maximum_quantity` int(10) NOT NULL default '0',
				`quantity_available` int(10) NOT NULL default '0',
				`quantity_sold` int(10) NOT NULL default '0',
				PRIMARY KEY  (`iid`),
				KEY `pid` (`pid`)
				) TYPE=MyISAM;";
		$_DB->execute($sql);
	}
	
	
}
?>











