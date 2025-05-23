<?php
//VersionInfo:Version[3.0.1]
// update

class Products {

    var $debug = false;
    var $navLinks = NULL;
    
    // --------------------------------------------------------------
    function __construct() {

	}
	
	
	// --------------------------------------------------------------
    function listProducts(){

		global $data;
        global $_DB;
        global $_Common;
        global $maxPerScreen;

		$where = "";
		$sku = NULL;
		$catid = NULL;
		$categoryName = NULL;
		$sortFld = NULL;
		$sql = null;

		if(!empty($_REQUEST['sku'])){
			$sku = trim($_REQUEST['sku']);
			$where = " sku = '$sku'";
			$catid = NULL;
			$_DB->queryVars['sku'] = $sku;
		}
		elseif(!empty($_REQUEST['mid'])){
			$mid = trim($_REQUEST['mid']);
			$where = " products.mid = '$mid'";
			$catid = NULL;
			$_DB->queryVars['mid'] = $mid;
			
		}
		elseif(!empty($_REQUEST['catid'])){
			$catid = $_REQUEST['catid'];
			// where for cat list query
			$where = " (categories.catid = '$catid' OR categories.parentid = '$catid')";
			// show category name at top of screen
			$cSql = "SELECT parentid,category_name,category_link FROM categories WHERE catid = '$catid'";
			$crs = $_DB->execute($cSql);
			$record = $_DB->fetchrow($crs, "ASSOC");
			if(count($record) > 0){
				if($record['parentid'] == 0){
					$categoryName = $record['category_name'];
				}
				else{
					$categoryName = $record['category_link'];
				}
			}
			$_DB->queryVars['catid'] = $catid;
		}

		if(!empty($_REQUEST['sortby'])){
			$sortFld = $_REQUEST['sortby'];
			$_DB->queryVars['sortby'] = $sortFld;
		}

		include_once("categories.inc");
		$catClass = new Categories();
		$categoryList = $catClass->makeCatSelect($catid,true);

		// run count sql
		$sql = "SELECT products.pid, product_categories.pid, categories.catid
				FROM products,product_categories,categories
				WHERE product_categories.pid = products.pid
				AND categories.catid = product_categories.catid";

		if($where != ""){
			$sql .= " AND $where";
		}

		// get a count
		$rs = $_DB->execute($sql);
		$rsCount = $_DB->numrows($rs);
		$_DB->free_result($rs);

		if($this->debug){
			$_Common->debugPrint($sql,"Count SQL");
			$_Common->debugPrint("Count: $rsCount");	
		}

		// Set limits for full sql
		list($start,$end,$limits) = $_DB->getLimits($rsCount,$maxPerScreen,"products.php");

		// run full query with limits
		$sql = "SELECT products.*, product_categories.*, categories.category_name, categories.category_link,categories.category_ids
				FROM products,product_categories,categories
				WHERE product_categories.pid = products.pid
				AND categories.catid = product_categories.catid";

		//$sql = "SELECT * FROM products WHERE LEFT(sku,3) = 'cak'";

		if($where != ""){
			$sql .= " AND $where";
		}

		if($sortFld){
			$sql .= " ORDER BY $sortFld";
		}
		else{
			$sql .= " ORDER BY categories.category_link";
		}

		// add screen limits
		$sql .= $limits;

		// add previous/next links
		$previousNextLinks = $_DB->previousNextLinks;

		if($this->debug){
			$_Common->debugPrint($sql);	
		}


		$vars = array('categoryList' => $categoryList,
					  'start'		=> $start,
					  'end'			=> $end,
					  'rsCount'		=> $rsCount,
					  'categoryName'=> $categoryName,
					  'catid'		=> $catid,
					  'previousNextLinks' => $previousNextLinks);


		$rs = $_DB->execute($sql);
		$cacheIds = array();
		while($row = $_DB->fetchrow($rs, "ASSOC")){

			$catnames = "";
			$lastCatNameLink = "";

			$cacheIds[$row['pid']] = $row['pid'];

			$cats = explode(":",$row['category_link']);
			$catids = explode(":",$row['category_ids']);

//			$_Common->debugPrint($cats);
//			$_Common->debugPrint($catids);

			for($i=0;$i<count($cats);$i++){
				$linkName = $cats[$i];
				$linkId = $catids[$i + 1];
				if($linkName != $lastCatNameLink){
					if($catnames == ""){
						$catnames .= "<a href=\"products.php?list=true&catid=$linkId\">$linkName</a>";
					}
					else{
						$catnames .= ", <a href=\"products.php?list=true&catid=$linkId\">$linkName</a>";
					}
				}
				$lastCatNameLink = $linkName;
			}				
			$row['catlinks'] = $catnames;
			
			$data[] = $row;
			//$_Common->debugPrint($row);
		}
		unset($row);
		$_SESSION['detailPidCache'] = array_keys($cacheIds);
		
		//$_Common->debugPrint($data);
		//exit;
		
		return $vars;

	}
	
	// --------------------------------------------------------------
    function editProducts($pid = ""){
		
		global $_CF;
        global $_Common;
        global $data;
        global $_DB;
        global $_Registry;

		if(!empty($_REQUEST['pid'])){
			$pid = $_REQUEST['pid'];
		}
		else{
			if(!empty($_REQUEST['edit'])){
				$this->showError("You did not select a product to add or modify?");
			}
		}
			// Get list of selected categories if pid is not null

		$selectedCats = array();
		if($pid){
			$sql = "SELECT catid FROM product_categories WHERE pid = '$pid'";
			$sCats = $_DB->getRecords($sql);
			if(count($sCats) > 0){
				foreach($sCats as $index=>$sFlds){
					$selectedCats[$sFlds['catid']] = $sFlds['catid'];
				}	
			}
		}
        
		include_once("categories.inc");
		$catClass = new Categories();
		$vars['parentSelectBox'] = $catClass->makeCatSelect($selectedCats,false,true);
        
        if(is_null($vars['parentSelectBox'])){
			$this->showError("No Categories have been defined. You must add categories before you can add products.");
		}

			// Get count of options
			
		$vars['optionCount'] = $_DB->getCount("options",NULL);

			// Now get products

		$sql = "SELECT products.*, product_custom.*
				FROM products LEFT JOIN product_custom ON products.pid=product_custom.custom_pid";

		$add = true;
		if($pid){
			$sql .= " WHERE products.pid = '$pid'";	
			$add = false;
		}
		$sql .= " LIMIT 1";

		//$_Common->debugPrint($sql);

		$data = array();
		global $fldProperties;
		$fldProperties = $_DB->getFieldProperties('products');
		$customFields = $_DB->getFieldProperties('product_custom');
		
		$vars['customFields'] = $customFields;
		$fldProperties = array_merge($fldProperties,$customFields);
		$inventoryItem = false;
		$inventoryOptions = false;

		$mid = 1;
		
		if($add){
			foreach($fldProperties as $fldName=>$properties){
				$data[$fldName] = $properties[4];
			}
		}
		else{
			$products = $_DB->getRecords($sql);
			if(isset($products[0]['custom_pid']) && $products[0]['pid'] == ""){
				$products[0]['pid'] = $products[0]['custom_pid'];
			}		
			if(count($products) > 0){
				$data = $products[0];
				if(!empty($data['mid'])){
					$mid = $data['mid'];
				}
			}
			if(isset($products[0]['inventory_item']) && $products[0]['inventory_item'] == "true"){
				$inventoryItem = true;
				if(isset($products[0]['inventory_options']) && $products[0]['inventory_options'] == "true"){
					$inventoryOptions = true;
				}
			}
		}

		$this->makeMfgSelect($mid);
		
		// load defaults and select each value
		$_REQUEST['edit'] = true;
		foreach($data as $key=>$value){
			$data[$key] = $_DB->getDefaultValues($key,$value);	
		}

		// get count of options
		if($pid){
			$vars['product_option_count'] = NULL;
			$oCount = $_DB->getCount("product_options","WHERE pid = '$pid'");
			if($oCount > 0){
				$vars['product_option_count'] = $oCount;
			}	
		}
		
		$vars['navLinks'] = NULL;
		$vars['inventoryData'] = array();
		$vars['isInventoried'] = false;
		$vars['optionsInventoried'] = false;
		
		if($pid){
			$vars['pid'] = $pid;
			$vars['navLinks'] = $this->detailPreviousNext($pid);
			$vars['haveInventoryClass'] = $_Registry->file_exists_incpath("inventory.inc");
			if($inventoryItem && $vars['haveInventoryClass']){
				$vars['isInventoried'] = true;
				if($inventoryOptions){
					$vars['optionsInventoried'] = true;
				}
				// Get inventory
				$inventory = $_Registry->loadClass("inventory");
				$vars['inventoryData'] = $inventory->GetInventory($pid,$inventoryOptions);
			}
		}
		
		if($this->debug){
			$_Common->debugprint($data);
			$_Common->debugprint($vars);
		}

	return $vars;

	}

	// --------------------------------------------------------------
    function updateProducts(){

		global $_CF;
		global $_DB;
		global $_Common;
		global $_Registry;

        // Check for the minimum fields (category, product_id, name)

		$requiredMessage = "A Category, product ID and product name are required.";

        $requiredFields = array("catid" => "No categories were selected?<br /><br />$requiredMessage",
								"sku" => "No sku number was entered?<br /><br />$requiredMessage",
								"name" => "No product name was entered?<br /><br />$requiredMessage");

		foreach($requiredFields as $fldnam=>$message){
			$fldvalue = NULL;
			if(!empty($_REQUEST[$fldnam])){
				if($fldnam == "catid" && count($_REQUEST[$fldnam]) == 0){
					$this->showError($message);
				}
				elseif($fldnam != "catid"){
					$fldvalue = trim($_REQUEST[$fldnam]);
					if($fldvalue == ""){
						$this->showError($message);
					}
				}
			}
			else{
				$this->showError($message);
			}
		}

		// Check the category lists
		$newCats = array();
        if(!empty($_REQUEST['catid']) && count($_REQUEST['catid']) > 0){
            $newCats = $_REQUEST['catid'];
            sort($newCats);
        }

        // Check the pid
        $pid = NULL;
        $modify = false;
        if(!empty($_REQUEST['pid'])){
            $pid = $_REQUEST['pid'];
            $modify = true;
        }

		// Check for images
        $images = array();
        if(count($_FILES) > 0){
			$upload = $_Registry->LoadClass("image.uploads");
            $upload->uploadImages($images);
            foreach($images as $fld=>$image){
				$_REQUEST[$fld] = $image;
			}
        }

			// Insert/Update product data

		$productFields = $_DB->getFieldProperties('products');
		$customFields = $_DB->getFieldProperties('product_custom');

		if($modify && $pid != 0 && $pid != NULL){
			// update category selections
			if(count($newCats) > 0){
				$sql = "DELETE FROM product_categories WHERE pid = '$pid'";
				$result = $_DB->execute($sql);
				foreach($newCats as $i=>$catid){
					$sql = "INSERT INTO product_categories (pid,catid) VALUES ('$pid','$catid')";
					$result = $_DB->execute($sql);
				}
			}     			
			// Update products
			if($updateFlds = $_DB->makeUpdateFields($productFields,'pid',$_REQUEST)){
				$sql = "UPDATE products SET $updateFlds WHERE pid = '$pid'";
				//print "<pre>$sql\n";
				$result = $_DB->execute($sql);
			}

			// check product_custom for records
			$customCount = $_DB->getCount('product_custom',"WHERE custom_pid = '$pid'");
			$_REQUEST['custom_pid'] = $pid;

			if(intval($customCount) > 0){
				// Update product_custom
				$updateFlds = $_DB->makeUpdateFields($customFields,'customid',$_REQUEST);
				if(trim($updateFlds) != ""){
					$sql = "UPDATE product_custom SET $updateFlds WHERE custom_pid = '$pid'";
					//print "<pre>$sql\n";
					$result = $_DB->execute($sql);
				}
			}
			else{
				// insert product_custom
				list($fields, $values) = $_DB->makeAddFields($customFields, 'customid', $_REQUEST);
				if(trim($fields) != ""){
					$sql = "INSERT INTO product_custom ($fields) VALUES ($values)";
					//print "<pre>$sql\n";
					$result = $_DB->execute($sql);
				}
			}
			
			if(!empty($_REQUEST['inventory_item'])){
				if($_Registry->file_exists_incpath("inventory.inc") && !empty($_REQUEST['inventory'])){
					// update the inventory numbers
					$inventory = $_Registry->loadClass('Inventory');
					if($_REQUEST['inventory_item'] == 'true'){
						// If we turned off the inventory_options, remove them and reset the counts.
						if($_REQUEST['inventory_options'] == 'false'){
							if($_DB->getCount('inventory', "WHERE pid = '$pid' AND odids != ''") > 0){
								$inventory->RemoveOptionsFromInventory($pid);
								$_REQUEST['inventory'] = array();
								$_REQUEST['inventory'][0]['pid'] = $pid;
								$_REQUEST['inventory'][0]['odids'] = "";
								$_REQUEST['inventory'][0]['name'] = $_REQUEST['name'];
								$_REQUEST['inventory'][0]['minimum_quantity'] = "0";
								$_REQUEST['inventory'][0]['maximum_quantity'] = "0";
								$_REQUEST['inventory'][0]['quantity_available'] = "0";
								$_REQUEST['inventory'][0]['quantity_sold'] = "0";
							}
							$inventory->SetInventory($pid);
						}
						else{
							$inventory->SetInventory($pid);
						}
					}
					elseif($_REQUEST['inventory_item'] == 'false'){
						if($_DB->getCount('inventory', "WHERE pid = '$pid'") > 0){
							$inventory->DeleteInventory($pid);
						}
					}
				}
			}
		}
		else{
			
			if(!empty($_REQUEST['price']) && empty($_REQUEST['retail_price'])){
				$_REQUEST['retail_price'] = $_REQUEST['price'];
			} 
			
			
			// Insert new product data
			list($fields, $values) = $_DB->makeAddFields($productFields, 'pid', $_REQUEST);
			$sql = "INSERT INTO products ($fields) VALUES ($values)";
			//print "<pre>$sql\n";
			$result = $_DB->execute($sql);
			$pid = $_DB->getInsertID('products','pid');
			$_REQUEST['pid'] = $pid;
			
			if($pid != 0 && $pid != NULL){
				
				// Custom Fields
				list($fields, $values) = $_DB->makeAddFields($customFields, 'customid', $_REQUEST);
				if(trim($fields) != ""){
					$sql = "INSERT INTO product_custom ($fields) VALUES ($values)";
					//print "<pre>$sql\n";
					$result = $_DB->execute($sql);
				}

				// Category links
				if(count($newCats) > 0){
					foreach($newCats as $i=>$catid){
						$sql = "INSERT INTO product_categories (pid,catid) VALUES ('$pid','$catid')";
						//print "<pre>$sql\n";
						$result = $_DB->execute($sql);
					}
				}
				
				// add the inventory numbers
				if(!empty($_REQUEST['inventory_item']) && $_REQUEST['inventory_item'] == 'true'){
					if($_Registry->file_exists_incpath("inventory.inc") && !empty($_REQUEST['inventory'])){
						$inventory = $_Registry->loadClass('Inventory');
						$inventory->SetInventory($pid);
					}
				}
			}				
		}
	
	}

	// --------------------------------------------------------------
    function deleteProducts(){

		global $_CF;
        global $_DB;
        global $_Common;
        global $_Registry;

		$pid = NULL;
		if(!empty($_REQUEST['pid'])){
			$pid = $_REQUEST['pid'];
		}

		if($pid){

			// Get images
			$sql = "SELECT name,thumbnail_image,fullsize_image FROM products WHERE pid = '$pid'";
			$rs = $_DB->execute($sql);
			$images = $_DB->fetchrow($rs, "ASSOC");
			$name = $images['name'];
			unset($images['name']);
			
			$imageDir = "../" . $_CF['images']['fullsize_images_directory'];
            $thumbDir = "../" . $_CF['images']['thumbnail_images_directory'];

			// Delete image files
			foreach($images as $fld=>$image){
				if($image != ""){
					if($fld == "thumbnail_image"){
						if(file_exists("$thumbDir/$image")){
							@unlink("$thumbDir/$image");
						}
					}
					else{
						if(file_exists("$imageDir/$image")){
							@unlink("$imageDir/$image");
						}
					}
				}
			}
			// Delete records in tables
    		$sql = "DELETE FROM product_categories WHERE pid = '$pid'";
			$result = $_DB->execute($sql);
        	$sql = "DELETE FROM products WHERE pid = '$pid'";
			$result = $_DB->execute($sql);
        	$sql = "DELETE FROM product_custom WHERE custom_pid = '$pid'";
			$result = $_DB->execute($sql);
			
			//clean related products
			$sql = "DELETE FROM related_products WHERE pid = '$pid'";
			$result = $_DB->execute($sql);
			$sql = "DELETE FROM related_products WHERE related_pid = '$pid'";
			$result = $_DB->execute($sql);

			$sql = "DELETE FROM inventory WHERE pid = '$pid'";
			$result = $_DB->execute($sql);
			
			$_Common->sendMessage("$name was deleted");
		}
		
		foreach($_REQUEST as $key=>$val){
			if($key == 'mid' || $key == 'catid'){
				continue;	
			}
			$_REQUEST[$key] = NULL;
			unset($_REQUEST[$key]);
		}
	}

	// --------------------------------------------------------------
	///<summary>
    ///	Makes an html select of manufactures from the database</summary>    
    function makeMfgSelect($id = null){
		
		global $_Common;
		global $_DB;
		global $mfgSelect;
		
		$sql = "SELECT DISTINCT mid,mfg_name FROM manufacturers ORDER BY mfg_name";
		$mfgs = $_DB->getRecords($sql,'mid');
		//$_Common->debugPrint($mfgs);
		if(count($mfgs) > 0){
			$mfgSelect = "<select name=\"mid\">\n";
			//$mfgSelect .= "\t<option value=\"0\">None</option>\n";
			foreach($mfgs as $mid=>$flds){
				$mfg = $flds['mfg_name'];
				if($id && $id == $mid){
					$mfgSelect .= "\t<option value=\"$mid\" selected>$mfg</option>\n";
				}
				else{
					$mfgSelect .= "\t<option value=\"$mid\">$mfg</option>\n";
				}
			}
			$mfgSelect .= "</select>\n";
		}
		else{
			$mfgSelect = "No manufacturers defined";
		}
	}

	
	// -------------------------------------------------------------------
	// Displays links to move through product results from detail screen
	function detailPreviousNext($pid){

		list($firstid,$previousid,$nextid,$lastid,$selectedIndex,$idCount) = $this->findInCache($pid);

		$detailLinks = "";

		if($idCount > 1) {

			if($firstid != ""){
				$fImage = "<img src=\"images/nav/arrow_first_on.gif\" border=\"0\">";
				$detailLinks = "<a href=\"products.php?edit=true&pid=$firstid\">$fImage</a>&nbsp;";
			}
			else{
				$detailLinks = "<img src=\"images/nav/arrow_first_off.gif\" border=\"0\">&nbsp;";
			}


			if($previousid != ""){
				$pImage = "<img src=\"images/nav/arrow_previous_on.gif\" border=\"0\">";
				$detailLinks .= "&nbsp;<a href=\"products.php?edit=true&pid=$previousid\">$pImage</a>&nbsp;";
			}
			else{
				$detailLinks .= "&nbsp;<img src=\"images/nav/arrow_previous_off.gif\" border=\"0\">&nbsp;";
			}

			$detailLinks .= "&nbsp; $selectedIndex of $idCount &nbsp;";

			if($nextid != ""){
				$nImage = "<img src=\"images/nav/arrow_next_on.gif\" border=\"0\">";
				$detailLinks .= "&nbsp;<a href=\"products.php?edit=true&pid=$nextid\">$nImage</a>&nbsp;";
			}
			else{
				$detailLinks .= "&nbsp;<img src=\"images/nav/arrow_next_off.gif\" border=\"0\">&nbsp;";
			}

			if($lastid != ""){
				$lImage = "<img src=\"images/nav/arrow_last_on.gif\" border=\"0\">";
				$detailLinks .= "&nbsp;<a href=\"products.php?edit=true&pid=$lastid\">$lImage</a>&nbsp;";
			}
			else{
				$detailLinks .= "&nbsp;<img src=\"images/nav/arrow_last_off.gif\" border=\"0\">&nbsp;";
			}
		}
		else{
			$detailLinks .= "&nbsp; $selectedIndex of $idCount &nbsp;";
		}

	return $detailLinks;
	}



	// -------------------------------------------------------------------
	// Used to find the current pid in the detail cache
	function findInCache($idToFind){

		global $g_Session;

		if(!empty($_SESSION['detailPidCache'])){
			$ids = $_SESSION['detailPidCache'];
			$idCount = count($ids);
			$firstid = 0;
			$previousid = 0;
			$selectedIndex = 0;
			$nextid = 0;
			$lastid = 0;
			$index = 0;
			if($ids[0] != $idToFind){
				$firstid = $ids[0];
			}
			if($ids[$idCount - 1] != $idToFind){
				$lastid = $ids[$idCount - 1];
			}
			foreach($ids as $i=>$id){
				if($id == $idToFind){
					$selectedIndex = $index + 1;
					if($index > 0){
						$previousid = $ids[$index - 1];
					}
					if($index < $idCount){
						if(isset($ids[$index + 1])){
							$nextid = $ids[$index + 1];
						}
					}
					break;
				}
				$index++;
			}
			return array($firstid,$previousid,$nextid,$lastid,$selectedIndex,$idCount);
		}
	}	


	// --------------------------------------------------------------
    //	Displays internal errors.
    function showError($error){
        print "<html><head><title>$error</title>";
        print "<link rel=\"stylesheet\" type=\"text/css\" href=\"stylesheets/admin.800.css\">";
        print "</head><body>";
        print "<div align=center>\n";
        print "<h4><br>Error</h4>\n";
        print "<p>$error</p>\n";
        print "<form><input type=button value=\"Go Back\" onClick=\"javascript:history.go(-1);\"></form>\n";
        print "</div>\n";
        print "</body></html>";
        exit;
    }
	
	
	
}

?>