<?php
//VersionInfo:Version[3.0.1]

class Categories {

    var $debug = false;
    // product count is used in products.inc to setup paging
    // when we display the list of products. We need a full
    // count even if the product is listed in more than one
    // category so all records display.
    var $productCount = 0;
    
    // --------------------------------------------------------------
    function __construct() {

	}

    // --------------------------------------------------------------
    // Displays a list of categories/subcategories and the related product
    // counts from the database
	function listCategories($forProducts = false){
		
		global $data;
		global $parents;
		global $subcats;
		global $_DB;
		global $_Common;

		//$cats = array();
		//$this->getCategories($cats);

        // get parent categories and product counts.
        $sql = "SELECT categories.catid, categories.category_name,categories.category_thumbnail,categories.category_ids,
					   categories.display_category, COUNT(product_categories.pid) AS count
                FROM categories LEFT JOIN product_categories ON categories.catid = product_categories.catid
                WHERE parentid = '0'
				GROUP BY categories.catid ORDER BY category_name";
		

        $parents = $_DB->getRecords($sql,'catid');
		
		//$_Common->debugPrint($parents);
		//exit; 

		if(count($parents) > 0){

			if(!$forProducts){
				$data[0] = "ok";
			}
			$subcats = array();

			foreach($parents as $catid=>$fields){

				$ids = $fields['category_ids'];
				$len = strlen($ids);
				
				$sql = "SELECT categories.catid,parentid,category_name,category_thumbnail,category_link,category_ids,
						categories.display_category, COUNT(product_categories.pid) AS count
						FROM categories LEFT JOIN product_categories ON categories.catid = product_categories.catid
						WHERE LEFT(category_ids,$len + 1) = '$ids:'
						GROUP BY categories.catid ORDER BY category_link";

				$records = $_DB->getRecords($sql);
				$subcats[$catid] = $records;
			}
		}
	}



	// --------------------------------------------------------------
    // Add or edit a category
    function editCategories($catid = NULL){
		
		global $data;
		global $_DB;
		global $_Common;

        $add = true;
        
        if(!$catid && !empty($_REQUEST['catid'])){
			$catid = $_REQUEST['catid'];
		}
		if($catid){
			$add = false;	
		}
        
		// If modify, get the parentid so we can select it
		$parid = 0;
		if(!empty($_REQUEST['parid'])){
			$parid = $_REQUEST['parid'];
		}

		$data = array();
		$data['parentSelectBox'] = "";

		// Set fields for edit screen
        if($add){
			$data['catid'] = "";
			$data['category_name'] = "";
			$data['category_description'] = "";
			$data['category_discount'] = "";
			$data['category_thumbnail'] = "";
            $data['pageTitle'] = "Add Category";
            $data['buttonName'] = "insert";
            $data['buttonText'] = "Add New";
            $selectedParent = null;
        }
        else{
			$sql = "SELECT * FROM categories WHERE catid = '$catid'";
			$data = $_DB->getRecord($sql);

			$selectedParent = $data['parentid'];
			
            $data['pageTitle'] = "Modify Category";
            $data['buttonName'] = "update";
            $data['buttonText'] = "Update"; 
		}

        // Get parent list and select the parent if this is a modify.
        $data['parentSelectBox'] = $this->makeCatSelect($selectedParent);

    }


	// --------------------------------------------------------------
    function updateCategories(){

        global $_CF;
        global $_Common;
        global $_DB;
        global $_Registry;

        // Check for category image upload first
        $images = array();
        if(count($_FILES) > 0){
			if(isset($_FILES['file-category_image']['error']) && $_FILES['file-category_image']['error'] == 0){
				$upload = $_Registry->LoadClass("image.uploads");
				$upload->uploadImages($images,true);
				if(isset($images['category_thumbnail'])){
					$_REQUEST['category_thumbnail'] = $images['category_thumbnail'];
				}
			}
        }

        // Set id defaults if not available
		if(empty($_REQUEST['parid'])){
			$_REQUEST['parentid'] = 0;
			$parentid = 0;
		}
		else{
			$_REQUEST['parentid'] = $_REQUEST['parid'];
			$parentid = $_REQUEST['parid'];
		}
		
		if(empty($_REQUEST['catid'])){
			$_REQUEST['catid'] = 0;
		}
        if(empty($_REQUEST['category_name']) || trim($_REQUEST['category_name']) == ""){
			$this->showError("You did not enter a category name to add or modify?");
        }
        if(empty($_REQUEST['category_name']) && strstr($_REQUEST['category_name'],':')){
			$this->showError("Colons are not allowed in the category name.");
        }

        if(isset($_REQUEST['parid']) && $_REQUEST['parid'] > 0){
            // not a root parent, get the category_links and category_ids for the given parent category
			$sql = "SELECT category_link,category_ids FROM categories WHERE catid = '$parentid'";
			$parentCatData = $_DB->getRecord($sql);
			$_REQUEST['category_link'] = $parentCatData['category_link'] . ':' . $_REQUEST['category_name'];
			$catIds = $parentCatData['category_ids'];
		}
		else{
			$_REQUEST['category_link'] = $_REQUEST['category_name'];
			$catIds = '0';
			$parentid = '0';
		}
		if(substr($_REQUEST['category_link'],0,1) == ':'){
			$_REQUEST['category_link'] = substr($_REQUEST['category_link'],1,strlen($_REQUEST['category_link']));
		}

		$fieldProperties = $_DB->getFieldProperties('categories');
		
		// Check if we are inserting new category or modifying an existing one.
		if(!empty($_REQUEST['insert'])){

            if($this->debug){
                print "Add new category\n\n";
            }
            // Test for an existing sub-category
            $found = false;
            $found = $this->testCategories($parentid,$_REQUEST['category_name'],true);
            if($found){
                $this->showError($found);
            }
            
			// create insert sql
			list($fields, $values) = $_DB->makeAddFields($fieldProperties, 'catid', $_REQUEST);
			
			$sql = "INSERT INTO categories ($fields) VALUES($values)";
			if($this->debug){
				$_Common->debugPrint($sql);
				//exit;	
			}
			$rs = $_DB->execute($sql);
	        $_DB->free_result($rs);

			// Get the new catid
	    	$catid = $_DB->getInsertID('categories','catid');

			$_REQUEST['catid'] = $catid;

			// merge the category ids
			$catIds .= ':' . $catid;
			if(substr($catIds,0,1) == ':'){
				$catIds = '0' . $catIds;
			}

			// Update the category ids
            $sql = "UPDATE categories SET category_ids = '$catIds' WHERE catid = '$catid'";
            $rs = $_DB->execute($sql);
            $_DB->free_result($rs);
            
			unset($_REQUEST['insert']);
        }
		else{
			
            // Modify existing catgory
            $catid = $_REQUEST['catid'];

            if($this->debug){
                print "Reset Parent of existing category and update text\n\n";
            }
            if($catIds == '0'){
				$catIds = '0:' . $catid;
			}
			if(substr($catIds,0,1) == ':'){
				$catIds = '0' . $catIds;
			}

			$idlist = explode(':',$catIds);
			$lastid = array_pop($idlist);
			if($lastid != $catid){
				// add this catid to the end of the list
		        $catIds .= ":$catid";
			}

			$_REQUEST['category_ids'] = $catIds;            
			
            $values = $_DB->makeUpdateFields($fieldProperties,'catid',$_REQUEST);
            $sql = "UPDATE categories SET $values WHERE catid = '$catid'";

			if($this->debug){
				$_Common->debugPrint($_REQUEST);
				//$_Common->debugPrint($fieldProperties);
				$_Common->debugPrint($sql);
				$_Common->debugPrint($catIds,"catids");
				//exit;	
			}

            $rs = $_DB->execute($sql);
            $_DB->free_result($rs);
            
			// if we changed the name we need to rebuild the
            // sub-category category_link and category_ids fields.
            $count = $_DB->getCount('categories',"WHERE parentid = '$catid'");
			if($count > 0){
				$this->updateSubCats($catid);
			}
			
			if(!empty($_REQUEST['update'])){
				unset($_REQUEST['update']);
			}
        }
	}
    
	// --------------------------------------------------------------------------
	// do a recursive loop through the subcats and update the category link
	// --------------------------------------------------------------------------
	function updateSubCats($catid){
	
		global $_Common;
		global $_DB;

		// get data for this category
		$thisCat = $_DB->getRecord("SELECT catid,parentid,category_link,category_ids FROM categories WHERE catid = '$catid'");
		//$_Common->debugPrint($thisCat,"This cat");		

		// get a list of subcats to loop through
		$subCats = $_DB->getRecords("SELECT catid,category_name FROM categories WHERE parentid = '$catid'",'catid');
		//$_Common->debugPrint($subCats,"Subcats");

		if(count($subCats) > 0){
			
			foreach($subCats as $cid=>$flds){
			
				$catids = $thisCat['category_ids'] . ':' . $cid;
				$catnames = $thisCat['category_link'] . ':' . $flds['category_name'];
				$sql = "UPDATE categories SET category_link = '$catnames', category_ids = '$catids' WHERE catid = '$cid'";
				$urs = $_DB->execute($sql);
				$_DB->free_result($urs);
			
				//$_Common->debugPrint($sql);
			
				// check for more subcats within this category
				$count = $_DB->getCount('categories',"WHERE parentid = '$cid'");
				if($count > 0){
					$this->updateSubCats($cid);
				}
			}
			
		}
	}

	// --------------------------------------------------------------------------
	function deleteCategories($catid = null){
		
		global $_Common,$_DB;
		
		if(!$catid && isset($_REQUEST['catid']) && is_numeric($_REQUEST['catid'])){
			$catid = intval($_REQUEST['catid']);
		}
		
		if($catid > 0){
			
			if(!empty($_REQUEST['recursive'])){

				// remove products
				$pids = array_keys($_DB->getRecords("SELECT pid FROM product_categories WHERE catid = '$catid'",'pid'));
				if(count($pids) > 0){
					$idList = "'" . join("','",$pids) . "'";
					$rs = $_DB->execute("DELETE FROM products WHERE pid IN($idList)");
					$_DB->free_result($rs);
					$rs = $_DB->execute("DELETE FROM product_options WHERE pid IN($idList)");
					$_DB->free_result($rs);
					$rs = $_DB->execute("DELETE FROM product_custom WHERE custom_pid IN($idList)");
					$_DB->free_result($rs);
					$rs = $_DB->execute("DELETE FROM product_categories WHERE catid = '$catid'");
					$_DB->free_result($rs);
				}
				$rs = $_DB->execute("DELETE FROM categories WHERE catid = '$catid'");
				$_DB->free_result($rs);
				
				// check for more subcats within this category and loop
				$subCats = $_DB->getRecords("SELECT catid,category_name FROM categories WHERE parentid = '$catid'",'catid');
				if(count($subCats) > 0){
					foreach($subCats as $cid=>$row){
						//$_Common->debugPrint($row['category_name']);
						$this->deleteCategories($cid);
					}
				}
			}
			else{
				$rs = $_DB->execute("DELETE FROM categories WHERE catid = '$catid'");
				$_DB->free_result($rs);
				$rs = $_DB->execute("DELETE FROM product_categories WHERE catid = '$catid'");
				$_DB->free_result($rs);
			}
		}
	}


	// --------------------------------------------------------------
    // Tests a category to see if it already exists in the database
    function testCategories($parid,$name,$add){

		global $_DB;

        $addError = "";

        $strText = "Category was not modified.";
        if($add){
            $strText = "Category was not added.";
        }

        // Check and see if this category with parent exists
        $catsearch = $_DB->getRecord("SELECT * FROM categories WHERE parentid = '$parid' AND category_name = '$name'");

        if(count($catsearch) > 0){
            if($parid == 0){
                $addError .= "$name already exists as a parent category. $strText\n";
            }
            else{
                $namesearch = $_DB->getRecord("SELECT category_name FROM categories WHERE catid = '$parid'");
                $parentName = $namesearch['category_name'];
                $addError .= "The '$name' sub-category already exists in the '$parentName' category. $strText\n";
            }
            return $addError;
        }

    return false;
    }


	// --------------------------------------------------------------
    // Makes an html select of parent categories with product counts from the database
    function makeCatSelect($selected = null,$forProductScreen = false,$forProductEdit = false){

		global $_Common;
		global $parents;
		global $subcats;


		if($forProductScreen){
			$this->listCategories(true);
			$catSelect = "\n<select name=\"catid\" onChange=\"this.form.submit();\">\n";
		}
		elseif($forProductEdit){
			$this->listCategories(true);
			$catSelect = "<select name=\"catid[]\" multiple size=\"10\">\n";	
		}
		else{
			$this->listCategories();
			$catSelect = "\n<select name=\"parid\">\n";
			$catSelect .= "\t<option value=\"0\">Save as new parent</option>\n";
		}

		if(count($parents) > 0){

			foreach($parents as $catid=>$row){

				$link = stripslashes($row['category_name']);
				$count = null;
				if($row['count'] > 0){
					$count = "(" . $row['count'] . ")";
					$this->productCount += $row['count'];
				}

				if(is_array($selected) && isset($selected[$catid])){
					$catSelect .= "\t<option value=\"$catid\" selected>$link $count</option>\n";
				}
				elseif(!is_array($selected) && !is_null($selected) && $catid == $selected){
					$catSelect .= "\t<option value=\"$catid\" selected>$link $count</option>\n";
				}
				else{
					$catSelect .= "\t<option value=\"$catid\">$link $count</option>\n";
				}
				
				if(isset($subcats[$catid])){
					
					foreach($subcats[$catid] as $i=>$flds){
						
						$category_link = str_replace($row['category_name'] . ':',"",$flds['category_link']);
						$id = $flds['catid'];
						$count = null;
						if($flds['count'] > 0){
							$count = "(" . $flds['count'] . ")";
							$this->productCount += $row['count'];
						}

						if(is_array($selected) && isset($selected[$id])){
							$catSelect .= "\t<option value=\"$id\" selected>&nbsp; - $category_link $count</option>\n";
						}
						elseif(!is_array($selected) && !is_null($selected) && $id == $selected){
							$catSelect .= "\t<option value=\"$id\" selected>&nbsp; - $category_link $count</option>\n";
						}
						else{
							$catSelect .= "\t<option value=\"$id\">&nbsp; - $category_link $count</option>\n";
						}
					}
				}
			}
			$catSelect .= "\t</select>\n";

			return $catSelect;
		}
		else{
			return null;
		}
    }

	// --------------------------------------------------------------
    ///	Displays internal errors.
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
