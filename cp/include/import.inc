<?php
//VersionInfo:Version[3.0.1]


class Import{

    var $debug = false;
	var $productId2Pid = array();
	var $isWindows = false;
	var $inventory_field = "user_1";

	// -------------------------------------------------------------------
    function __construct(){
		if(isset($_SERVER['windir'])){
			$this->isWindows = true;
		}
	}

	// --------------------------------------------------------------
	function doUploads(){

		global $_CF;
		global $uploadStatus;

		$uploadErrors = array(0=>"There is no error, the file uploaded with success", 
							  1=>"The uploaded file exceeds the upload_max_filesize directive in php.ini", 
							  2=>"The uploaded file exceeds the MAX_FILE_SIZE directive",
							  3=>"The uploaded file was only partially uploaded",
							  6=>"Missing a temporary folder");
		
		$validExtensions = array("gif"	=> 1,
								 "jpg"	=> 1,
								 "htm"	=> 1,
								 "html"	=> 1,
								 "dat"	=> 1,
								 "txt"	=> 1);
		$dir = getcwd();

		// include FTP in case we need it.
		include_once("ftp.inc");
		$ftp = new Ftp();
		$ftpRoot = $_CF['ftp']['document_root'] . "/cp";
		$ftp->ChDir($ftpRoot);
		$inDir = false;
		
		foreach($_FILES as $fldName=>$fData){

			$fName = $fData['name'];

			if(isset($fData['error']) && $fData['error'] > 0){
				$fError = intval($fData['error']);
				if(isset($uploadErrors[$fError])){
					$uploadStatus[$fName]['ERROR'] = $uploadErrors[$fError];
				}
				continue;
			}
			if(isset($fData['tmp_name']) && ($fData['tmp_name'] == "none" || trim($fData['tmp_name']) == "")){
				continue;
			}
			if(!isset($fData['name']) || trim($fData['name']) == ""){
				continue;
			}
			
			$pathinfo = pathinfo($fName);
			
			$ext = NULL;
			if(!empty($pathinfo['extension'])){
				$ext = strtolower($pathinfo['extension']);
				if($ext != "dat"){
					$dir = "../";
					if(!$inDir){
						$ftp->ChDir("../");
						$inDir = true;
					}
				}
				else{
					$dir = "database";
					if(!is_dir($dir)){
						if(!@mkdir($dir)){
							$ftp->Mkdir(null, $dir,'0777');
						}	
					}
					if(!$inDir){
						$ftp->ChDir("./database");
						$inDir = true;
					}
				}
			}
			if($ext && isset($validExtensions[$ext])){
				if(@move_uploaded_file($fData['tmp_name'], "$dir/$fName")){
					if(isset($uploadStatus[$fName]['ERROR'])){
						unset($uploadStatus[$fName]['ERROR']);
					}
					$uploadStatus[$fName]['status'] = "$fName Uploaded Successfully";
					$_REQUEST[$fldName] = $fName;
				}
				else{
					if(!$ftp->Upload($fData['tmp_name'],$fName)){
						$ftp->Close();
						$uploadStatus[$fName]['ERROR'] = "$fName Upload Failed: " . $ftp->error;
						$_REQUEST[$fldName] = "";	
					}
					else{
						$uploadStatus[$fName]['status'] = "$fName Uploaded Successfully";
						$_REQUEST[$fldName] = $fName;
						if(isset($uploadStatus[$fName]['ERROR'])){
							unset($uploadStatus[$fName]['ERROR']);
						}
					}
				}
			}
			else{
				$uploadStatus[$fName]['ERROR'] = "Failed: Invalid file name: $fName";
				$_REQUEST[$fldName] = "";	
			}
		}
		return $uploadStatus;
	}
	
	
	// --------------------------------------------------------------
	function importProducts(){
		
		global $_DB;
		global $_Common;
		global $_CR;

		$counter = 0;
		$dataFile = getcwd() . "/database/products.dat";
		$dataFile = str_replace('\\','/',$dataFile);
		
		if(file_exists($dataFile)){
			
			// Remove old products
			if(isset($_REQUEST['overwrite_products']) && $_REQUEST['overwrite_products'] == "true"){
				$_DB->execute("TRUNCATE categories");
				$_DB->execute("TRUNCATE product_categories");
				$_DB->execute("TRUNCATE products");
				$_DB->execute("TRUNCATE product_custom");
				$_DB->execute("TRUNCATE inventory");
				$_DB->execute("OPTIMIZE TABLE `categories` , `product_categories` , `products`, `product_custom`, `inventory`");	
			}

			// always dump these tables
			$_DB->execute("TRUNCATE product_import");
			$_DB->execute("TRUNCATE option_import");
			$_DB->execute("OPTIMIZE TABLE `product_import`, `option_import`");

			// load the new data
			if($this->isWindows){
				$sql = "LOAD DATA LOCAL INFILE '$dataFile'
						INTO TABLE `product_import`
						FIELDS TERMINATED BY '|' LINES TERMINATED BY '\r\n'";
			}
			else{
				$sql = "LOAD DATA LOCAL INFILE '$dataFile'
						INTO TABLE `product_import`
						FIELDS TERMINATED BY '|' LINES TERMINATED BY '\n'";
			}

			$rs = $_DB->execute($sql,null);
			$_DB->free_result($rs);

			if($_DB->error && $_DB->error == '1148'){
				$importData = file($dataFile);
				foreach($importData as $importData=>$line){
					$line = str_replace("'",'`',$line);
					$fields = explode('|',$line);
					if(trim(strtolower($fields[0])) == 'product_id' || trim($fields[0]) == ""){
						continue;	
					}
					if(count($fields) == 16){
						$row = "'" . join("','",$fields) . "'";
						$sql = "INSERT INTO product_import VALUES($row)";
						$rs = $_DB->execute($sql,null,true);
						$_DB->free_result($rs);
					}
				}
			}
		}
		
		$count = $_DB->getCount("product_import");

		if($count > 0){

			// Do some cleanup on the "NA" fields from the products.dat
			$importFields = $_DB->getFieldProperties("product_import");
			$cleanup = $_DB->getRecords("SELECT * FROM product_import");
			foreach($cleanup as $c=>$flds){
				foreach($flds as $n=>$v){
					
					$cleanup[$c][$n] = str_replace('&#39;',"`",$v);
					
					if($v == "NA"){
						$cleanup[$c][$n] = '';
					}
				}
				$sku = $flds['sku'];
				$uFlds = $_DB->makeUpdateFields($importFields,'sku',$cleanup[$c]);
				$uSQL = "UPDATE product_import SET $uFlds WHERE sku = '$sku'";
				$rs = $_DB->execute($uSQL);
				$_DB->free_result($rs);
			}


			// load the custom fields table and check to make sure the fields are there
			$customFields = array_keys($_DB->getFieldProperties("product_custom"));
			$Fields212 = array('user_1','user_2','user_3','user_4','options');
			foreach($Fields212 as $j=>$fld){
				if(!in_array($fld,$customFields)){
					$sql = "ALTER TABLE `product_custom` ADD `$fld` VARCHAR( 255 ) NOT NULL";
					$rs = $_DB->execute($sql);
					$_DB->free_result($rs);
				}	
			}

			$categoryData = array();

			$date = date("Y-m-d");

			// Products table
			$sql = "INSERT INTO products (`sku`,`price`,`retail_price`,`name`,`description`,`fullsize_image`,`thumbnail_image`,`size`,`weight`,`link_page`,`link_text`,`last_modified`)
					SELECT product_import.sku,product_import.price,product_import.price,product_import.name,product_import.description,
						   product_import.fullsize_image,product_import.thumbnail_image,product_import.size,product_import.weight,
						   product_import.link_page,product_import.link_text,'$date' FROM product_import
					WHERE LOWER(product_import.sku) != 'product_id' AND product_import.sku != ''";

			$rs = $_DB->execute($sql);
			$_DB->free_result($rs);

			sleep(1);

			// fix fields for mysql5
			$_DB->execute("ALTER TABLE `product_custom` CHANGE `options` `options` varchar(255) NULL");
			$_DB->execute("ALTER TABLE `product_custom` CHANGE `detail_page_title` `detail_page_title` TEXT NULL");
			$_DB->execute("ALTER TABLE `product_custom` CHANGE `detail_meta_keywords` `detail_meta_keywords` TEXT NULL");
			$_DB->execute("ALTER TABLE `product_custom` CHANGE `detail_meta_description` `detail_meta_description` TEXT NULL");

			// Products_custom
			$sql = "INSERT INTO product_custom (`custom_pid`,`user_1`,`user_2`,`user_3`,`user_4`,`options`)
					SELECT products.pid,
						   product_import.user_1,product_import.user_2,product_import.user_3,product_import.user_4,product_import.options
					FROM products,product_import
					WHERE product_import.sku = products.sku";

			$rs = $_DB->execute($sql);
			$_DB->free_result($rs);

			// Categories
			$data = $_DB->getRecords("SELECT products.pid,product_import.category
									  FROM products,product_import
									  WHERE product_import.sku = products.sku");

			$this->importCategories($data);

			$counter = $_DB->getCount("products");
			
			$this->updateInventory();
			
		}
		return "$counter products were imported successfully";
	}
	
	// -------------------------------------------------------------------
	function importOptions(){

		global $_DB;
		global $_Common;
		global $_CR;

		$counter = 0;
		$dataFile = getcwd() . "/database/options.dat";
		$dataFile = str_replace('\\','/',$dataFile);
		
		if(file_exists($dataFile)){
			
			// Remove old Options
			if(isset($_REQUEST['overwrite_options']) && $_REQUEST['overwrite_options'] == "true"){
				$_DB->execute("TRUNCATE options");
				$_DB->execute("TRUNCATE option_details");
				$_DB->execute("TRUNCATE product_options");
				$_DB->execute("OPTIMIZE TABLE `option_details`, `options`, `product_options`");	
			}

			$_DB->execute("TRUNCATE option_import");
			$_DB->execute("OPTIMIZE TABLE `option_import`");
			// load the new data
			$sql = "LOAD DATA LOCAL INFILE '$dataFile'
					INTO TABLE `option_import`
					FIELDS TERMINATED BY '|' LINES TERMINATED BY '$_CR'";
			$rs = $_DB->execute($sql,null,false);
			$_DB->free_result($rs);

			if($_DB->error && $_DB->error == '1148'){
				$importData = file($dataFile);
				foreach($importData as $importData=>$line){
					$line = trim(str_replace("'",'`',$line));
					$fields = explode('|',$line);
					if(trim(strtolower($fields[0])) == 'product_id' || trim($fields[0]) == ""){
						continue;	
					}
					if(count($fields) == 9){
						$row = "'" . join("','",$fields) . "'";
						$sql = "INSERT INTO option_import VALUES($row)";
						$rs = $_DB->execute($sql,null,true);
						$_DB->free_result($rs);
					}
				}
			}
			else{
				$rs = $_DB->execute("DELETE FROM option_import WHERE LOWER(sku) = 'product_id'");
				$_DB->free_result($rs);
			}

			$count = $_DB->getCount("option_import");

			$this->productId2Pid = $_DB->getRecords("SELECT DISTINCT `sku`,`pid` FROM products",'sku');

			if($count > 0 && count($this->productId2Pid) > 0){
				
				$data = $_DB->getRecords("SELECT * FROM option_import WHERE LOWER(sku) != 'product_id' AND sku != ''");

				$optionFields = $_DB->getFieldProperties("options");
				$detailFields = $_DB->getFieldProperties("option_details");
				$productOptionFields = $_DB->getFieldProperties("product_options");

				$counter = 0;
				$option_order = 1;
				$lastProductId = NULL;
				$lastOptionNumber = NULL;
				
				foreach($data as $i=>$fields){
					
					$record = array();	
					$record['product_id'] = $fields['sku'];
					$record['number']	= $fields['number'];
					$record['format']	= $fields['format'];
					$record['name']		= $fields['name'];
					$record['value']	= $fields['value'];
					$record['price']	= $fields['price'];
					$record['weight']	= $fields['weight'];
					$record['text']		= $fields['text'];
					$record['required']	= $fields['required'];

					switch($record['format']){
						case "1":
							$record['format'] = "select box";
							break;
						case "2":
						case "2a":
							$record['format'] = "radio buttons";
							break;
						case "3":
							$record['format'] = "text box";
							break;
						default:
							$record['format'] = "select box";
							break;
					}
					
					if($record['product_id'] != $lastProductId || $record['number'] != $lastOptionNumber){
						
						$record['sequence'] = 1;
						
						if($record['required'] == ""){
							$record['required'] = 'false';	
						}
						
						// insert to options and get oid
						list($fields, $values) = $this->makeInsertFields($record,$optionFields);
						$sql = "INSERT INTO options ($fields) VALUES ($values)";
						$_DB->execute($sql);
						$oid = $_DB->getInsertID("options","oid");
						$record['oid'] = $oid;
						$counter++;
						
						if($record['product_id'] == $lastProductId && $record['number'] != $lastOptionNumber){
							$record['sequence']++;
						}
						
						if(isset($this->productId2Pid[$record['product_id']])){
							$record['pid'] = $this->productId2Pid[$record['product_id']]['pid'];
							// insert into product_options to tie it together
							list($fields, $values) = $this->makeInsertFields($record,$productOptionFields);
							$sql = "INSERT INTO product_options ($fields) VALUES ($values)";
							$_DB->execute($sql);
						}
						else{
							$productID = $record['product_id'];
							$pidData = $_DB->getRecord("SELECT pid FROM products WHERE sku = '$productID' LIMIT 1");
							if(!empty($pidData['pid'])){
								$record['pid'] = $pidData['pid'];
								// insert into product_options to tie it together
								list($fields, $values) = $this->makeInsertFields($record,$productOptionFields);
								$sql = "INSERT INTO product_options ($fields) VALUES ($values)";
								$_DB->execute($sql);
							}
						}
						
						// set the last group we saw
						$lastProductId = $record['product_id'];
						$lastOptionNumber = $record['number'];
						$option_order = 1;
					}
					else{
						$option_order++;	
					}
					
					$record['oid'] = $oid;
					$record['option_order']	= $option_order;
					// insert to option details
					list($fields, $values) = $this->makeInsertFields($record,$detailFields);
					$sql = "INSERT INTO option_details ($fields) VALUES ($values)";
					$_DB->execute($sql);
				}
			}
			
			// update option description
			$details = $_DB->getRecords("SELECT * FROM option_details");
			
			$descriptions = array();
			foreach($details as $j=>$flds){
				$flds['text'] = str_replace("<br>","",$flds['text']);
				$flds['text'] = str_replace("'","&#39;",$flds['text']);
				$descriptions[$flds['oid']][] = $flds['text'];
			}
			foreach($descriptions as $oid=>$f){
				$desc = join(', ',$f);
				$sql = "UPDATE options SET description = '$desc' WHERE oid = $oid";
				$_DB->execute($sql);
			}
			
		}
		return "$counter options were imported successfully";
	}

	# -------------------------------------------------------------------
	function importCategories($data){
		
		global $_DB;
		global $_Common;

		$catFields = $_DB->getFieldProperties("categories");
		if(isset($catFields['category_description'])){
			$_DB->execute("ALTER TABLE `categories` CHANGE `category_description` `category_meta_description` TEXT");
		}
		if(!isset($catFields['category_meta_keywords'])){
			$_DB->execute("ALTER TABLE `categories` ADD `category_meta_keywords` TEXT NULL");
			$_DB->execute("ALTER TABLE `categories` ADD `category_page_title` TEXT NULL");
		}

		$categoryList = array();
		
		foreach($data as $i=>$fields){
		
			if(strtolower($fields['category']) == "na" || trim($fields['category']) == ""){
				continue;	
			}
	        
			$cat = trim($fields['category']);
			if(strstr($cat,",")){
				$subCats = array();
				$subCats = explode(",",$cat);
				foreach($subCats as $j=>$pCat){
					$pCat = trim($pCat);
					$categoryList[$pCat][] = trim($fields['pid']);
				}
			}
			else{
				$categoryList[$cat][] = trim($fields['pid']);
			}
		}
	    
		ksort($categoryList);

		//$_Common->debugPrint($categoryList);
		//exit;

		$parentid = 0;
		$catid = 0;

		$doneParents = array();

		foreach($categoryList as $cat=>$pids){
	       
			if(trim($cat) != ""){ 
				if(strstr($cat,':')){
					
					$subCats = array();
					$subCats = explode(':',$cat);

					// load parent once.
					$pCat = trim($subCats[0]);
					unset($subCats[0]);
					
					$found = $_DB->getRecord("SELECT parentid,catid FROM categories WHERE category_name = '$pCat'");										
					if(count($found) > 0){
						$parentid = $found['catid'];
					}
					else{
						if(!isset($doneParents[$pCat])){
							$parentid = '0';
							$sql = "INSERT INTO categories (parentid,category_name,category_meta_description,category_meta_keywords,category_page_title,category_link) VALUES ('$parentid','$pCat','$pCat','','','$pCat')";
							$_DB->execute($sql);
							$parentid = $_DB->getInsertID('categories','catid');
							$sql = "UPDATE `categories` SET category_ids = CONCAT(parentid, ':', catid) WHERE catid = '$parentid'";
							$_DB->execute($sql);
							$doneParents[$pCat] = 1;
						}
					}
		            
					foreach($subCats as $j=>$sCat){
						
						$sCat = trim($sCat);
						$subfound = $_DB->getRecord("SELECT catid FROM categories WHERE category_name = '$sCat'");
						if(count($subfound) > 0){
							$subCatID = $subfound['catid'];
						}
						else{
							$sql = "INSERT INTO categories (parentid,category_name,category_meta_description,category_meta_keywords,category_page_title,category_link) VALUES ('$parentid','$sCat','$sCat','','','$cat')";
							$_DB->execute($sql);						
							$subCatID = $_DB->getInsertID('categories','catid');
							$sql = "UPDATE `categories` SET category_ids = CONCAT('0:', parentid, ':', catid) WHERE catid = '$subCatID'";
							$_DB->execute($sql);
						}						

						foreach($pids as $k=>$id){
							$productsToCategories[$id][$subCatID] = $subCatID;
						}
					}
				}
				else{
					$found = $_DB->getRecord("SELECT catid FROM categories WHERE category_name = '$cat'");										
					if(count($found) > 0){
						$newID = $found['catid'];
					}
					else{
						$parentid = '0';
						$sql = "INSERT INTO categories (parentid,category_name,category_meta_description,category_meta_keywords,category_page_title,category_link) VALUES ('$parentid','$cat','$cat','','','$cat')";
						$_DB->execute($sql);
						$newID = $_DB->getInsertID('categories','catid');
						$sql = "UPDATE `categories` SET category_ids = CONCAT(parentid, ':', catid) WHERE catid = '$newID'";
						$_DB->execute($sql);
					}
					foreach($pids as $k=>$id){
						$productsToCategories[$id][$newID] = $newID;
					}
				}
			}
		}

		// Link products to categories
		foreach($productsToCategories as $pid=>$catids){
			foreach($catids as $k=>$cid){
				$sql = "INSERT INTO product_categories (pid,catid) VALUES ('$pid','$cid')";
				$_DB->execute($sql);
			}
		}
	}


	// -------------------------------------------------------------------
	// similar to makeAddFields except we may want to add the id field
	function makeInsertFields($dbfields,$fldProperties = array()){

		$fields = "";
		$values = "";

		foreach($dbfields as $dbFldName=>$fldValue){

			if(count($fldProperties) > 0 && !isset($fldProperties[$dbFldName])){
				continue;	
			}

			$fldValue = trim(preg_replace("|\'|","''",$fldValue));
	        
			if(trim($fldValue) == "NA"){
				$fldValue = "";	
			}

			if($fields == ""){
				$fields .= "`$dbFldName`";
				$values .= "'$fldValue'";
			}
			else{
				$fields .= ", `$dbFldName`";
				$values .= ", '$fldValue'";
			}

		}
		return array($fields, $values);
	}

	// -------------------------------------------------------------------
	function updateInventory(){
		
		global $_DB;
		$invFld = $this->inventory_field;
		
		$invData = $_DB->getRecords("SELECT products.name,product_custom.custom_pid,product_custom.user_1
									 FROM products,product_custom 
									 WHERE product_custom.custom_pid = products.pid AND product_custom.user_1 != ''");
		foreach($invData as $i=>$fields){
			$pid = $fields['custom_pid'];
			$available = $fields[$invFld];
			if(strtolower($available) == "sold out"){
				$available = 0;
			}
			elseif(!is_numeric($available)){
				$available = 0;
			}
			$name = $fields['name'];
			$sql = "INSERT INTO inventory (`pid`,`oids`,`odids`,`name`,`quantity_available`)
									VALUES('$pid','','','$name','$available')";
			$_DB->execute($sql);
		}
	}

}
?>