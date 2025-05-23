<?php

class Category_menu{

    var $debug = false;
    var $cats = array();
	var $cacheMenu = true;
	var $cacheTimeInSeconds = 900;	
	
	// --------------------------------------------------------------
	function __construct(){
		
	}
	
	// --------------------------------------------------------------
	function getCategories(){
		
		global $_CF;
		global $_DB;
        global $_Common;
        global $cats;
        global $_Registry;

		$haveInventory = $_Registry->file_exists_incpath("inventory.inc");
        
		// category list
		$sortFld = "category_name";
		if(!empty($_CF['product_display']['sort_menu_by_fields'])){
			$sortFld = $_CF['product_display']['sort_menu_by_fields'];
		}
		
		$sql = "SELECT parentid,catid,category_name FROM categories WHERE display_category = 'true'	ORDER BY parentid,$sortFld";
		$this->cats = $_DB->getRecords($sql,'catid');

		// sub-categories counts
		$idList = "'" . join("','",array_keys($this->cats)) . "'";
		$sql = "SELECT parentid,COUNT(catid) AS total FROM categories WHERE parentid IN($idList) AND display_category = 'true' GROUP BY catid";
		$rs = $_DB->execute($sql);
		while($row = $_DB->fetchrow($rs,"ASSOC")){
			$this->cats[$row['parentid']]['subcount'] = $row['total'];
		}
		$_DB->free_result($rs);

		// count of products with display flag on for each category
		$sql = "SELECT product_categories.catid, COUNT(product_categories.pid) as total
				FROM products,product_categories,manufacturers
				WHERE products.pid = product_categories.pid
				AND manufacturers.mid = products.mid
				AND product_categories.catid IN($idList)
				AND products.display_product = 'true'
				AND manufacturers.display_mfg = 'true'
				GROUP BY product_categories.catid";

		//$_Common->debugPrint($sql);
				
		$rs = $_DB->execute($sql);
		while($row = $_DB->fetchrow($rs,"ASSOC")){
			
			//$_Common->debugPrint($row);

			if($haveInventory){
				// check inventory items and reset product counts
				$catid = $row['catid'];
				$iSQL = "SELECT DISTINCT product_categories.pid, products.inventory_item, products.display_when_sold_out,
							SUM(inventory.quantity_available) AS available
							FROM (product_categories,products)
							LEFT JOIN inventory ON product_categories.pid = inventory.pid
							WHERE product_categories.catid = '$catid'
							AND products.pid = product_categories.pid
							GROUP BY product_categories.pid";
							
				$inventory = $_DB->getRecords($iSQL);
				
				foreach($inventory as $j=>$flds){
					if($flds['inventory_item'] == 'true'){
						if($flds['available'] == 0 && $flds['display_when_sold_out'] == 'false'){
							$row['total']--;
						}
					}
				}
			}
			
			//$_Common->debugPrint($row);
			$this->cats[$row['catid']]['prodcount'] = $row['total'];
		}
		$_DB->free_result($rs);
        
    }
	
	// --------------------------------------------------------------
	function getMenuList(){
		
		global $_DB;
        global $_Common;
		global $cats;
		
        $result = NULL;
		
		if($this->cacheMenu){
			// This caches the category menu for 10 minutes so it does
			// not take so much horespower to keep loading it.
			$cacheTime = $this->cacheTimeInSeconds;
			$sql = "SELECT TIME_TO_SEC(CURTIME()),TIME_TO_SEC(`timestamp`),category_list
					FROM category_menu 
					WHERE (TIME_TO_SEC(CURTIME()) - TIME_TO_SEC(`timestamp`)) < $cacheTime";
			$record = $_DB->getRecord($sql);
			
			if(count($record) > 0){
				// clear out old time from previous day and reload
				if($record['time_to_sec(curtime())'] < $record['time_to_sec(`timestamp`)']){
					$record = array();
				}
			}
			
			if(count($record) == 0){
				if($this->debug){
					$_Common->debugPrint("Reloading category list");
				}
				$_DB->execute("TRUNCATE TABLE category_menu");
				$this->getCategories();
				$categoryList = serialize($this->cats);
				$sql = "INSERT INTO category_menu (`category_list`,`timestamp`) VALUES('$categoryList',CURTIME())";
				$_DB->execute($sql);
			}
			else{
				if($this->debug){
					$diff = floor(($record['time_to_sec(curtime())'] - $record['time_to_sec(`timestamp`)']) / 60);
					$_Common->debugPrint("$diff minutes", "time since category list was reloaded");
				}
				$this->cats = unserialize($record['category_list']);
			}
		}
		else{
			//$_DB->execute("TRUNCATE TABLE category_menu");
			$this->getCategories();
		}
		return $this->cats;
	}
}
?>