<?php
class Display{

	var $debug = false;

	var $breadCrumbs = NULL;
	var $catVars = array();
	var	$catQueryResult = 0;
	var $detail = false;
	var $end = 0;
	var $favoritesListTitle = "Your Favorites";
	var $inventory = array();
	var $inventoryPids = array();
	var $navLinks = NULL;
	var $metaDescription = NULL;
	var $metaKeywords = NULL;
	var $parentCategory = NULL;
	var $pidList = array();
	var $queryVars = array();
	var $records = array();
	var $recordCount = 0;
	var $resultPidList = array();
	var	$runCatQuery = false;
	var $searchCriteria = array();
	var $subCategories = array();
	var $showCatImages = false;
	var $start = 0;

	function __construct(){

	}

	// --------------------------------------------------------------
	function runQuery(){
		
		global $cart;
		global $_CF;
		global $_Common;

		if(!empty($_REQUEST['detail'])){
			$this->detail = true;
		}

		$records = array();
		$this->getSearchCriteria();
		$this->queryDatabase();

//$_Common->debugPrint($this->records);
//exit;


		if(count($this->records) > 0){
			// If not detail, set session['last_page'] or create different links
			if(!$this->detail){
				$this->setLastPage();
			}
			elseif($this->detail && (count($this->pidList) > 0 ||
				  (!empty($_SESSION['detailPidCache']) && count($_SESSION['detailPidCache']) > 0))){
					
				// create detail navigation links
				$this->detailPreviousNext();	
			}
			$this->setParentCategory();
			$this->makeMetaTags();
			$this->setBreadCrumbs();
			$this->getImages();
			$this->getOptions();
			$this->getRelatedItems();
			$this->getInventory();
		}
		
		if($this->debug){
			$_Common->debugPrint($this->queryVars,"Query Variables");
			$_Common->debugPrint($this->searchCriteria,"Search Criteria");
			$_Common->debugPrint($this->subCategories,"Sub Category List");
			$_Common->debugPrint($this->pidList,"PID List");	
			$_Common->debugPrint($this->records,"Product Records");
			exit;
		}

		$qURL = $this->setLastPage(true);
		$cart->updateLastPage($qURL);
		
	}

    // -------------------------------------------------------------------
    // Function name
    //   getSearchCriteria
    // General description
    //   Checks the form fields to setup the criteria for a product search
    // Inputs
    //   $_REQUEST
    // Outputs
    //   criteria array
    // Modified Date
    //   12/28/2004
    //
    function getSearchCriteria(){	

		global $_CF;
		global $_DB;
        global $_Common;

			// Do categories first - NULL out the "all" entries

		$catid = (empty($_REQUEST['catid']) || strtolower($_REQUEST['catid']) == "all" ? NULL : trim($_REQUEST['catid']));
		$catName = (empty($_REQUEST['cat']) || strtolower($_REQUEST['cat']) == "all" ? NULL : trim($_REQUEST['cat']));
		$subCat = (empty($_REQUEST['subcat']) || strtolower($_REQUEST['subcat']) == "all" ? NULL : trim($_REQUEST['subcat']));

			// Long versions
		
		if(!$catName){
			$catName = (empty($_REQUEST['category']) || strtolower($_REQUEST['category']) == "all" ? NULL : trim($_REQUEST['category']));
		}
		if(!$subCat){
			$subCat = (empty($_REQUEST['subcategory']) || strtolower($_REQUEST['subcategory']) == "all" ? NULL : trim($_REQUEST['subcategory']));
		}
		
		if($catid || $catName || $subCat){

			if($subCat && strstr($subCat,":")){
				$this->queryVars['subcat'] = $subCat;
				$catFields = explode(":",$subCat);
				$catid = array_pop($catFields);
				$this->catVars['categories.catid'] = $catid;
				$this->addToSearchCriteria('categories.category_ids',$subCat);
			}
			else{
				if($catid && strstr($catid,":")){
					$this->queryVars['catid'] = $catid;
					$catFields = explode(":",$catid);
					$catid = array_pop($catFields);
					$this->catVars['categories.catid'] = $catid;
					$this->addToSearchCriteria('categories.category_ids',$subCat);
				}										
				elseif($catid){
					$this->queryVars['catid'] = $catid;
					$this->catVars['categories.catid'] = $catid;
					$this->addToSearchCriteria('categories.catid',$catid);
				}
				elseif($catName || $subCat){
					if($subCat){
						$this->queryVars['subcat'] = $subCat;
						$data = $_DB->getRecord("SELECT catid FROM categories WHERE category_link = '$subCat'");
						if(isset( $data['catid'])){
							$catid = $data['catid'];
							$this->catVars['categories.catid'] = $catid;
							$this->addToSearchCriteria('categories.catid',$catid);
						}
					}
					elseif($catName){
						$this->queryVars['category'] = $catName;
						$data = $_DB->getRecord("SELECT catid FROM categories WHERE category_link = '$catName'");
						if(isset( $data['catid'])){
							$catid = $data['catid'];
							$this->catVars['categories.catid'] = $catid;
							$this->addToSearchCriteria('categories.catid',$catid);
						}
					}
				}
				
				if($catName){
					$this->queryVars['category'] = $catName;
				}
				
			}
			$this->querySubCategories();
		}
		else{
			$this->catVars['categories.catid'] = 0;
			if(empty($_REQUEST['keywords'])){
				$this->querySubCategories();
			}
		}
		
        $priceRange = false;
        $lowPrice = 0;
        $highPrice = 9999999;
        $and = false;
        $exactMatch = false;

			// Get list of db fields from products and product_custom to search on.

		$fields = $_DB->getFieldProperties('products');
		$productFields = $fields;
		$customFields = $_DB->getFieldProperties('product_custom');
		$fields = array_merge($fields,$customFields);


        if(!empty($_REQUEST['product'])){
            $_REQUEST['sku'] = trim($_REQUEST['product']);
        }
		
        // Price search
        if(!empty($_REQUEST['low_price'])){
            $lowPrice = $_Common->format_price($_REQUEST['low_price']);
			$this->queryVars['low_price'] = $lowPrice;
            $priceRange = true;
        }
        if(!empty($_REQUEST['high_price'])){
            $highPrice = $_Common->format_price($_REQUEST['high_price']);
			$this->queryVars['high_price'] = $highPrice;            
            $priceRange = true;
        }

        // Boolean "and" for Keyword search
        if(!empty($_REQUEST['and'])){
            $and = $_REQUEST['and'];
            if($and != "1" && $and != "yes" && $and != "true" && $and != ""){
                $and = false;
            }
            else{
				$this->queryVars['and'] = 1;
				$and = true; 	
			}
        }

        // Boolean "exact_match" for Keyword search
        if(!empty($_REQUEST['exact_match'])){
            $exactMatch = $_REQUEST['exact_match'];
            if($exactMatch == "1" || $exactMatch == "yes" || $exactMatch == "true" || $exactMatch == "on"){
                $exactMatch = true;
                $this->queryVars['exact_match'] = 1; 
            }
        }
        
		// Keyword search
        if(!empty($_REQUEST['keywords'])){

			$this->queryVars['keywords'] = trim($_REQUEST['keywords']); 

			// Test the server to see if we can do boolean searches
			// Only available in 4.x.x versions of mysql.

            $mySQLVersion = mysql_get_server_info();

            $booleanOK = false;
			if(version_compare($mySQLVersion, "4.0.1", ">=")){
                $booleanOK = true;
            }

            $keywords = preg_replace("|,|"," ",trim($_REQUEST['keywords']));
            $keywords = preg_replace("|  |"," ",$keywords);
			$keywordArray = array();
			
            if($exactMatch){
				$keywordArray = explode(" ",$keywords);
				if(count($keywordArray) > 1){
	                $keywords = '"' . $keywords . '"';
	            }
            }
            elseif($and && $booleanOK){
                $keywordArray = explode(" ",$keywords);
                if(count($keywordArray) > 1){
					$keywordList = "";
	                foreach($keywordArray as $word){
		                $keywordList .= " +" . trim($word);
			        }
				    $keywords = trim($keywordList);
				}
            }

			$keywordFields = 'products.name,products.description';
			$blnHaveOtherFields = false;
			if(!empty($_CF['product_display']['keyword_fields'])){
				$keywordFields = trim($_CF['product_display']['keyword_fields']);
				// you cannot add fields to the full text (MATCH below) index. It will not find any results
				$blnHaveOtherFields = true;
			}

            if(strlen($keywords) > 3 && !$blnHaveOtherFields){
                if($and || $exactMatch){
                    if($booleanOK){
						$this->addToSearchCriteria("MATCH ($keywordFields) AGAINST ('$keywords' IN BOOLEAN MODE)");
                    }
                    else{
                        $this->addToSearchCriteria("MATCH ($keywordFields) AGAINST ('$keywords')");
                    }
                }
                else{
					$this->addToSearchCriteria("MATCH ($keywordFields) AGAINST ('$keywords')");
                }
            }
            else{
				$searchStr = "(";
				foreach(explode(',',$keywordFields) as $i=>$fld){
					if($i==0){
						$searchStr .= "LOCATE('$keywords', $fld)";
					}
					else{
						$searchStr .= " OR LOCATE('$keywords', $fld)";
					}
				}
				$searchStr .= ")";
				$this->addToSearchCriteria($searchStr);
            }
        }        

        if(isset($this->pidList[$catid]) && count($this->pidList[$catid]) > 0){
            sort($this->pidList[$catid]);
            $pidList = "'" . join("','",$this->pidList[$catid]) . "'";
            $this->addToSearchCriteria("products.pid IN ($pidList)");
		}
        
        // manufacturers
        if(!empty($_REQUEST['mid'])){
			$mid = trim($_REQUEST['mid']);
			$this->queryVars['mid'] = $mid;
			$this->addToSearchCriteria("manufacturers.mid = '$mid'");            
        }
        
        
            // Check each DB field and see if we have a matching form
            // field to search on. If so, add it to the search criteria

        foreach($fields as $name=>$type){

			if($name == "page"){
				continue;	
			}

            if($priceRange && $name == "price"){
                $this->addToSearchCriteria("(price BETWEEN $lowPrice AND $highPrice)");
            }
            else{
				$likeName = $name . "_like";

                if(!empty($_REQUEST[$name])){
                    $val = trim($_REQUEST[$name]);
                    
                    if($val != ""){
						$this->queryVars[$name] = $val;	
					}
                    
                    if($name == "sku" && strstr($val,",") && count($this->pidList) == 0){
                        $skus = explode(',',$val);
                        sort($skus);
                        $pidList = "'" . join("','",$skus) . "'";
                        $this->addToSearchCriteria("products.sku IN ($pidList)");
                    }
                    else{
                        if($val != ""){
							if($name == 'pid' && strstr($val,",")){
								$pids = explode(',',$val);
								sort($pids);
								$pidList = "'" . join("','",$pids) . "'";
								$this->addToSearchCriteria("products.pid IN ($pidList)");
								if(isset($_REQUEST['favorites'])){
									$this->parentCategory = $this->favoritesListTitle;
								}
								else{
									$this->parentCategory = "Search Result";
								}
							}
							else{
								if(isset($productFields[$name])){
									$name = "products." . $name;	
								}
								if(isset($customFields[$name])){
									$name = "product_custom." . $name;	
								}							
								$this->addToSearchCriteria($name,$val);
							}
                        }
                    }
                }
                // Allow users to add "like" statements to search field name. This includes the % signs if they use like
                elseif(!empty($_REQUEST[$likeName])){
					$val = trim($_REQUEST[$likeName]);
					if($val != ""){
						$this->queryVars[$likeName] = $val;

						if(isset($productFields[$name])){
							$name = "products." . $name;	
						}
						if(isset($customFields[$name])){
							$name = "product_custom." . $name;	
						}
						
						$this->addToSearchCriteria("LOCATE('$val', $name)");
					}                    
				}                
            }
        }

		$_SESSION['queryVars'] = $this->queryVars;
	}

	// --------------------------------------------------------------
	function addToSearchCriteria($key,$val = NULL,$isCat = false){
		
        if(count($this->searchCriteria) > 0){
			if($val){
	            $this->searchCriteria[] = "AND $key = '$val'";
	        }
	        else{
				$this->searchCriteria[] = "AND $key";
			}
        }
        else{
			if($val){
				$this->searchCriteria[] = "WHERE $key = '$val'";
			}
			else{
				$this->searchCriteria[] = "WHERE $key";
			}
		}
	}
	
	// --------------------------------------------------------------
	function querySubCategories(){
		
		global $_DB;
        global $_Common;

		$subCatSQL = "SELECT categories.catid, categories.parentid, categories.category_name, categories.category_thumbnail, categories.category_link,
					  Count(product_categories.pid) AS product_count
					  FROM categories LEFT JOIN product_categories ON categories.catid = product_categories.catid";

		$catid = NULL;
		if(isset($this->catVars['categories.catid'])){
			$catid = $this->catVars['categories.catid'];
			$subCatSQL .= " WHERE (categories.catid = '$catid' OR categories.parentid = '$catid')";
		}
		$subCatSQL .= " GROUP BY categories.catid, categories.category_name";
		$subCatSQL .= " ORDER BY categories.category_name";

		$rs = $_DB->execute($subCatSQL);

		if($_DB->numrows($rs) > 0){
			
			$row = array();
			while($row = $_DB->fetchrow($rs, "ASSOC")){

				$countCatid = $row['catid'];

				if($row['product_count'] > 0){
					$pidSQL = "SELECT pid FROM product_categories WHERE catid = '$countCatid'";
					$rs2 = $_DB->execute($pidSQL);
					$row2 = array();
					while($row2 = $_DB->fetchrow($rs2, "ASSOC")){
						$this->pidList[$countCatid][] = $row2['pid'];
					}
					$_DB->free_result($rs2);
				}
				
				if(trim($row['category_thumbnail']) != ""){
					$this->showCatImages = true;
				}
				
				//$_Common->debugPrint($row);

				if($catid != $row['catid']){
					$this->subCategories[$row['catid']] = array("category_link" => $row['category_link'],
																"category_name" => $row['category_name'],
																"category_thumbnail" => $row['category_thumbnail'],
																"product_count"	=>	$row['product_count']);
				}
				else{
					$this->parentCategory = $row['category_link'];
				}
			}
		}
		$_DB->free_result($rs);

	}
    // -------------------------------------------------------------------
    // Function name
    //   queryDatabase
    // General description
    //   Connects to the db class and queries for records
    // Inputs
    //   ref to records
    // Outputs
    //   result records
    // Modified Date
    //   01/14/04
    //
    function queryDatabase(){

        global $_CF;
		global $_DB;
        global $_Common;
        global $_Registry;

		$haveInventory = $_Registry->file_exists_incpath("inventory.inc");

		$count = 0;

		if(count($this->searchCriteria) > 0){
			
			$where = join(" ", $this->searchCriteria);

			// Show only products marked for display
			if($haveInventory){
				$sql = "SELECT categories.*, products.*, product_custom.*,manufacturers.*,
						CASE products.inventory_item
							WHEN 'true' THEN SUM(inventory.quantity_available)
							ELSE 0
						END AS available_inventory,
						CASE products.inventory_item
							WHEN 'true' THEN inventory.minimum_quantity
							ELSE 0
						END AS minimum_quantity
						FROM (categories,product_categories,products,manufacturers)
						LEFT JOIN product_custom ON products.pid = product_custom.custom_pid
						LEFT JOIN inventory ON products.pid = inventory.pid
						$where
						AND products.display_product != 'false'
						AND categories.catid = product_categories.catid
						AND products.mid = manufacturers.mid
						AND manufacturers.display_mfg = 'true'
						AND product_categories.pid = products.pid
						GROUP BY products.pid
						HAVING (products.inventory_item = 'false'
							OR (products.inventory_item = 'true' AND available_inventory > 0)
							OR (products.inventory_item = 'true' AND (available_inventory = 0 OR available_inventory = '')
							AND products.display_when_sold_out = 'true'))";
			}
			else{
				$sql = "SELECT categories.*, products.*, product_custom.*,manufacturers.*
						FROM (categories,product_categories,products,manufacturers) 
						LEFT JOIN product_custom ON products.pid=product_custom.custom_pid
						$where
						AND products.display_product != 'false'
						AND categories.catid = product_categories.catid
						AND products.mid = manufacturers.mid
						AND manufacturers.display_mfg = 'true'
						AND product_categories.pid = products.pid GROUP BY products.pid";
			}

			$cRS = $_DB->execute($sql);
			$count = $_DB->numrows($cRS);
			$_DB->free_result($cRS);
			
			if($this->debug){
				$_Common->debugPrint($sql,"record count: $count");
			}
		}

		if($count > 0){
					
			if(!empty($_REQUEST['sort_by'])){
				$sortby = $_REQUEST['sort_by'];
				$productFlds = $_DB->getFieldProperties('products');
				$customFlds = $_DB->getFieldProperties('product_custom');
				$catFlds = $_DB->getFieldProperties('categories');									
				if(isset($productFlds[$sortby])){
					$sql .= " ORDER BY products.$sortby";
				}
				elseif(isset($customFlds[$sortby])){
					$sql .= " ORDER BY product_custom.$sortby";
				}
				elseif(isset($catFlds[$sortby])){
					$sql .= " ORDER BY categories.$sortby";
				}
			}
			else{
				if(isset($_CF['product_display']['sort_by_fields'])){
					$sortFlds = $_CF['product_display']['sort_by_fields'];
					$sql .= " ORDER BY $sortFlds";
				}
				else{
					$sql .= " ORDER BY category_name,products.name";
				}
			}

			if(!$this->detail){
				// create a cache of the pids so we can add detail
				// previous/next links.
				$crs = $_DB->execute($sql);
				$cacheRows = array();
				$cacheIds = array();
				while($cacheRows = $_DB->fetchrow($crs, "ASSOC")){
					if(isset($cacheRows['pid'])){
						$cacheIds[] = $cacheRows['pid'];	
					}
				}
				$_DB->free_result($crs);
				$_SESSION['detailPidCache'] = $cacheIds;
				
				if($this->debug){
					$_Common->debugPrint($cacheIds,"CacheIds");
				}
				
				unset($cacheIds);
			}
			
			// Add limits for only the ones we want to see
			list($start,$end,$limits) = $this->getLimits($count);

			$this->start = $start;
			$this->end = $end;
			$this->recordCount = $count;

			$sql .= $limits;

			if($this->debug){
				$_Common->debugPrint("$start - $end of $count Total Records");
				$_Common->debugPrint("<b>Limited SQL:</b> $sql");
			}

			// check for category or customer discounts
			$discounts = $_Registry->LoadClass("Discounts");

			$rs = $_DB->execute($sql);
			if($_DB->numrows($rs) > 0){
				$rows = array();
				while($row = $_DB->fetchrow($rs, "ASSOC")){
					$row['in_cart'] = 'false';
					if($this->isInCart($row['sku'])){
						$row['in_cart'] = 'true';
					}
					$this->records[] = $row;
					if(isset($row['pid'])){
						$this->resultPidList[] = $row['pid'];	
					}
				}
				$_DB->free_result($rs);
				// check for category or customer discounts
				$discounts->calculateProductDiscounts($this->records);
			}
		}
    }	
	// --------------------------------------------------------------
	function getLimits($recordCount){
		
		global $_CF;

		if($this->detail){
			$maxRows = 1;
		}
		elseif(isset($_REQUEST['view_all']) && $_REQUEST['view_all'] == "true"){
			$maxRows = $recordCount;
			$_SESSION['view_all'] = true;
			if(isset($_REQUEST['view_all_hits'])){
				$_SESSION['view_all_hits'] = $_REQUEST['view_all_hits'];
			}
		}
		elseif(isset($_REQUEST['view_all']) && $_REQUEST['view_all'] == "false"){
			$maxRows = $_CF['product_display']['max_rows_to_return'];
			$_SESSION['view_all'] = false;
			if(isset($_SESSION['view_all_hits'])){
				$_REQUEST['hits'] = $_SESSION['view_all_hits'];
			}
		}
		elseif(isset($_SESSION['view_all']) && $_SESSION['view_all'] == true){
			$maxRows = $recordCount;
			$_REQUEST['view_all'] = true;
			if(isset($_SESSION['view_all_hits'])){
				$_REQUEST['view_all_hits'] = $_SESSION['view_all_hits'];
			}
		}
		else{
			$maxRows = $_CF['product_display']['max_rows_to_return'];
		}
		
		if(!$this->detail && !empty($_REQUEST['per_page'])){
			$maxRows = $_REQUEST['per_page'];
			if($_REQUEST['per_page'] == "all"){
				$maxRows = $recordCount;
			}
			$this->queryVars['per_page'] = $maxRows;
		}
		
		$hits = 0;
		if(!empty($_REQUEST['hits']) && $_REQUEST['hits'] > 0){
			$hits = $_REQUEST['hits'];
			$this->queryVars['hits'] = $hits;
			$_SESSION['hits'] = $hits;
		}
		if($this->detail){
			$hits = 0;
		}
		
		$start = $hits + 1;
		$end = $maxRows + $hits;
		if($end > $recordCount){
			$end = $recordCount;
		}
		
		// create navigation links
		$this->createPreviousNextLinks($hits,$recordCount,$maxRows);
		
		// Set SQL Limit
		if($this->detail){
			$limit = " LIMIT 1";
		}
		else{
			$limit = " LIMIT $hits,$maxRows";
		}
		
		return array($start,$end,$limit);
	}

	// --------------------------------------------------------------
	function setBreadCrumbs(){
		
		$keywords = (empty($_REQUEST['keywords']) ? NULL : trim($_REQUEST['keywords']));
		$breadCrumbs = "";
		if(isset($this->records[0])){
			$breadCrumbs = $keywords ? $keywords : $this->getBreadCrumbs($this->records[0]);
		}	
			
		$this->breadCrumbs = $breadCrumbs;
	}

	
	// --------------------------------------------------------------
	function getBreadCrumbs($record){
		
		global $_CF;
		global $_Common;
		
		$breadCrumbLink = "";
		$breadcrumbs = explode(':',$record['category_link']);
		$breadCrumbIds = explode(':',$record['category_ids']);

		$catLink = $record['category_link'];

		//$_Common->debugPrint($breadcrumbs);
	
		$catid = "";
		foreach($breadCrumbIds as $i=>$cid){
			$catid = $cid;
			if($i > 0 && isset($breadcrumbs[$i -1])){
				$link = $breadcrumbs[$i -1];
				
//marcello $link = ucwords(strtolower(str_replace("_"," ",$link))); 
				$link = ucwords(str_replace("_"," ",$link));

				$count = $this->checkProductCountForCategory($catid);
				
				if($breadCrumbLink == ""){
					if($count > 0){
						$breadCrumbLink .= "<a class=\"breadcrumbs\" href=\"products.php?catid=$catid&amp;category=$catLink\">$link</a>";
					}
					else{
						$breadCrumbLink .= $link;
					}
				}
				else{
					if($count > 0){
						$breadCrumbLink .= " > <a class=\"breadcrumbs\" href=\"products.php?catid=$catid&amp;category=$catLink\">$link</a>";
					}
					else{
						$breadCrumbLink .= " > $link";
					}
				}
			}
		}
		return $breadCrumbLink;
	}

	// --------------------------------------------------------------
	function checkProductCountForCategory($catid){
		
        global $_Common,$_DB,$_Registry;

		$haveInventory = $_Registry->file_exists_incpath("inventory.inc");		
		
		// count of products with display flag on for each category
		$sql = "SELECT product_categories.catid, COUNT(product_categories.pid) as total
				FROM products,product_categories,manufacturers
				WHERE products.pid = product_categories.pid
				AND manufacturers.mid = products.mid
				AND product_categories.catid = '$catid'
				AND products.display_product = 'true'
				AND manufacturers.display_mfg = 'true'
				GROUP BY product_categories.catid";
				
		$rs = $_DB->execute($sql);				

		$count = 0;
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

			$count = $row['total'];
		}
		$_DB->free_result($rs);
				
		return $count;
	}

	
	// --------------------------------------------------------------
	function makeMetaTags(){
		
		global $_CF;
		global $_Common;
		global $_DB;

		//$_Common->debugPrint($this->records[0]);

		if(!empty($this->records[0])){
			if(isset($_REQUEST['pid']) && isset($_REQUEST['detail'])){
				
				if(!empty($this->records[0]['detail_page_title'])){
					$this->pageTitle = trim($this->records[0]['detail_page_title']);
				}

				if(!empty($this->records[0]['detail_meta_keywords'])){
					$this->metaKeywords = trim($this->records[0]['detail_meta_keywords']);
				}
				else{
					$this->makeKeywordMetaTags();
				}
				
				if(!empty($this->records[0]['detail_meta_description'])){
					$this->metaDescription = trim($this->records[0]['detail_meta_description']);
				}
				else{
					$this->metaDescription = trim($this->records[0]['name']);
				}
				
			}
			elseif(!empty($_REQUEST['catid']) || !empty($_REQUEST['category'])){
				// all records will have the same category meta information
				
				if(!empty($this->records[0]['category_page_title'])){
					$this->pageTitle = trim($this->records[0]['category_page_title']);
				}
				
				if(!empty($this->records[0]['category_meta_keywords'])){
					$this->metaKeywords = trim($this->records[0]['category_meta_keywords']);
				}
				else{
					$this->makeKeywordMetaTags();
				}
				
				if(!empty($this->records[0]['category_meta_description'])){
					$this->metaDescription = trim($this->records[0]['category_meta_description']);
				}
				elseif($this->metaDescription == ''){
					$this->metaDescription = trim($this->records[0]['category_name']);
				}

			}
			else{
				$this->makeKeywordMetaTags();
				if(isset($_REQUEST['keywords'])){
					$this->pageTitle = "Search Results for keywords: " . trim($_REQUEST['keywords']);
				}
			}
		}
		if($this->debug){
			$_Common->debugprint($this->pageTitle,"Page Title");
			$_Common->debugprint($this->metaDescription,"META Description");
			$_Common->debugprint($this->metaKeywords,"META Keywords");
		}
	}
	
	// --------------------------------------------------------------
	function makeKeywordMetaTags(){
		
		global $_CF;
		global $_Common;

		$uniqueCats = array();
		$uniqueKeywords = array();
		$productNames = array();
		foreach($this->records as $i=>$fields){
			
			if(!empty($fields['category_meta_description'])){
				// remove quotes so it does not effect the meta html tag.
				$fields['category_meta_description'] = str_replace('_'," ",$fields['category_meta_description']);
				$fields['category_meta_description'] = str_replace('"',"&quot;",$fields['category_meta_description']);
				$fields['category_meta_description'] = strip_tags($fields['category_meta_description']);
			}
			else{
				if(!empty($fields['category_link'])){
					$fields['category_meta_description'] = join(", ",explode(":",$fields['category_link']));
				}
				elseif(!empty($fields['category_name'])){
					$fields['category_meta_description'] = $fields['category_name'];
				}
			}
			
			// Make categories and keywords unique so we don't spam them
			$uniqueCats[$fields['category_meta_description']] = 1;

			$uniqueKeywords[$fields['category_meta_description']] = 1;
			
			$fields['description'] = str_replace('"',"&quot;",$fields['description']);
			$fields['description'] = str_replace('[',"",$fields['description']);
			$fields['description'] = str_replace(']',"",$fields['description']);
			$fields['description'] = str_replace('(',"",$fields['description']);
			$fields['description'] = str_replace(')',"",$fields['description']);
			$fields['description'] = str_replace('.',"",$fields['description']);
			$fields['description'] = strip_tags($fields['description']);
			
			$names = explode(" ",trim($fields['description']));
			foreach($names as $j=>$name){
				if(strlen(trim($name)) > 4 && trim($name) != ','){
					$productNames[trim($name)] = 1;
				}
			}
		}
		foreach($productNames as $word=>$flag){
			if(strlen($word) > 3){
				$uniqueKeywords[$word] = 1;
			}
		}

		$metaDescription = join(" ",array_keys($uniqueCats));
		$metaKeywords = join(", ",array_keys($uniqueKeywords));
		
		if(isset($_REQUEST['detail'])){
			$metaDescription = $this->records[0]['name'];
		}

		if($this->debug){
			$_Common->debugprint($metaDescription,"META Description");
			$_Common->debugprint($metaKeywords,"META Keywords");
		}
		
		$this->metaDescription = $metaDescription;
		$this->metaKeywords = $metaKeywords;
	}
		
	// --------------------------------------------------------------
	function setLastPage($return = false){

		$last_page = "";

		// Build the last_page if not a detail query
		foreach($this->queryVars as $key=>$val){
			if($last_page == ""){
				$last_page = "products.php?$key=$val";
			}
			else{
				$last_page .= "&$key=$val";
			}
		}
		if($return){
			return $last_page;
		}
		else{
			$_SESSION['last_page'] = $last_page;
		}
	}
	
	// -------------------------------------------------------------------
	function createPreviousNextLinks($hits,$TotalRecords,$maxRows){
		
		if($hits < 0){
			$hits = 0;
		}
		
		$linkCount = ($TotalRecords / $maxRows);
		$CurrentPage = ($hits / $maxRows) + 1;
		$CurrentPage = ceil($CurrentPage);
		
		$TotalResultPages = intval($TotalRecords / $maxRows);
		if(($TotalRecords % $maxRows) != 0) {
			$TotalResultPages += 1;
		}
		
		$LoPage = $CurrentPage - 1;
		$HiPage = $CurrentPage + 8;
		
		if($LoPage<1) {
			$LoPage =1;
			if($TotalResultPages > 10) {
				$HiPage = 10;
			}
			else {
				$HiPage = $TotalResultPages;
			}
		}
		
		if($HiPage > $TotalResultPages) {
			$HiPage = $TotalResultPages;
			$LoPage = $HiPage - 9;
			if($LoPage < 1) {
				$LoPage = 1;
			}
		}
		
		$PreviousNextLinks = "";
		$queryString = "";
		
		foreach($this->queryVars as $key=>$val){
			if($key == "hits"){
				continue;
			}
			if($queryString == ""){
				$queryString = "$key=$val";
			}
			else{
				$queryString .= "&amp;$key=$val";
			}
		}
		
		if(isset($_SESSION['view_all']) && $_SESSION['view_all'] == true){
			$PreviousNextLinks .= "<a class=\"pnlinks\" href=\"products.php?$queryString&amp;view_all=false\">View Pages</a>";
		}
		else{
		
			if($TotalResultPages > 1) {
				
				$PreviousNextLinks .= "<a class=\"pnlinks\" href=\"products.php?$queryString&amp;view_all=true&amp;view_all_hits=$hits\">View All</a>&nbsp; | &nbsp;";
				
				if($CurrentPage > 1){
					$pHits = ($hits - $maxRows);
					$PreviousNextLinks .= "<img src=\"images/nav/arrow_left.gif\">&nbsp;";
					$PreviousNextLinks .= "<a class=\"pnlinks\" href=\"products.php?$queryString&amp;hits=$pHits\">Previous</a>&nbsp;";
				}
				
				$PreviousNextLinks .= "&nbsp;";
				
				$formHits = $hits;
				$highCount = 0;

				for($i = $LoPage; $i <= $HiPage; $i++){
					$hits = (($i - 1) * $maxRows) - $maxRows;
					if($hits < 0){
						$hits = 0;
					}
					
					$nHits = ($hits + $maxRows);
					$highCount = $nHits;
					
					if($i == $CurrentPage){
						if($i == $HiPage){
							$PreviousNextLinks .= "<span class=\"pncurrent\">$i</span> ";
						}
						else{
							$PreviousNextLinks .= "<span class=\"pncurrent\">$i</span> - ";
						}
					}
					else{
						if($i == 1){
							$PreviousNextLinks .=
							"<a class=\"pnlinks\" href=\"products.php?$queryString&amp;hits=0\">$i</a> - ";
						}
						elseif($i > $CurrentPage){
							if($i == $HiPage){
								$PreviousNextLinks .=
								"<a class=\"pnlinks\" href=\"products.php?$queryString&amp;hits=$nHits\">$i</a> ";
							}
							else{
								$PreviousNextLinks .=
								"<a class=\"pnlinks\" href=\"products.php?$queryString&amp;hits=$nHits\">$i</a> - ";
							}
						}
						else{
							if($i == $HiPage){
								$PreviousNextLinks .=
								"<a class=\"pnlinks\" href=\"products.php?$queryString&amp;hits=$nHits\">$i</a> ";
							}
							else{
								$PreviousNextLinks .=
								"<a class=\"pnlinks\" href=\"products.php?$queryString&amp;hits=$nHits\">$i</a> - ";
							}
						}
					}
				}
				if($CurrentPage != $TotalResultPages){
					$nHits = ($formHits + $maxRows);
					$PreviousNextLinks .= "&nbsp;<a class=\"pnlinks\" href=\"products.php?$queryString&amp;hits=$nHits\">Next</a>";
					$PreviousNextLinks .= "&nbsp;<img src=\"images/nav/arrow_right.gif\">";
				}
	         
	         
			}
		}
		
		$this->navLinks = $PreviousNextLinks;
	}

	// -------------------------------------------------------------------
	// Displays links to move through product results from detail screen
	function detailPreviousNext(){

		global $_Common;

		$detailLinks = "";

		$pid = null;
		if(isset($_REQUEST['pid'])){
			$pid = $_REQUEST['pid'];
		}

		//if(empty($_SESSION['detailPidCache']) || !$pid){
		//	return $detailLinks;
		//}

		list($firstid,$previousid,$nextid,$lastid,$selectedIndex,$idCount) = $this->findInCache($pid);

		if($idCount > 1) {

			if($firstid != ""){
				$fImage = "<img src=\"images/nav/arrow_first_on.gif\" border=\"0\">";
				$detailLinks = "<a href=\"products.php?detail=1&pid=$firstid\">$fImage</a>&nbsp;";
			}
			else{
				$detailLinks = "<img src=\"images/nav/arrow_first_off.gif\" border=\"0\">&nbsp;";
			}


			if($firstid && $previousid != ""){
				$pImage = "<img src=\"images/nav/arrow_previous_on.gif\" border=\"0\">";
				$detailLinks .= "&nbsp;<a href=\"products.php?detail=1&pid=$previousid\">$pImage</a>&nbsp;";
			}
			else{
				$detailLinks .= "&nbsp;<img src=\"images/nav/arrow_previous_off.gif\" border=\"0\">&nbsp;";
			}

			$detailLinks .= "&nbsp; $selectedIndex of $idCount &nbsp;";

			if($nextid != ""){
				$nImage = "<img src=\"images/nav/arrow_next_on.gif\" border=\"0\">";
				$detailLinks .= "&nbsp;<a href=\"products.php?detail=1&pid=$nextid\">$nImage</a>&nbsp;";
			}
			else{
				$detailLinks .= "&nbsp;<img src=\"images/nav/arrow_next_off.gif\" border=\"0\">&nbsp;";
			}

			if($lastid != ""){
				$lImage = "<img src=\"images/nav/arrow_last_on.gif\" border=\"0\">";
				$detailLinks .= "&nbsp;<a href=\"products.php?detail=1&pid=$lastid\">$lImage</a>&nbsp;";
			}
			else{
				$detailLinks .= "&nbsp;<img src=\"images/nav/arrow_last_off.gif\" border=\"0\">&nbsp;";
			}
		}

		$this->navLinks = $detailLinks;
	}

	// -------------------------------------------------------------------
	// Used to find the current pid in the detail cache
	function findInCache($idToFind){

		global $_Common;
		
		$ids = array();
		
		if(isset($_SESSION['detailPidCache'])){
			$ids = $_SESSION['detailPidCache'];
		}
		
		$idCount = count($ids);

		if($idCount == 0){
			return array(0,0,0,0,0,0);	
		}

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

		//$_Common->debugPrint($ids,$idToFind);

		if(!in_array($idToFind,$ids)){
			$idCount = 0;
		}
		else{
			foreach($ids as $i=>$id){

				if($id == $idToFind){
					$selectedIndex = $index + 1;
					if($index > 0){
						$previousid = $ids[$index - 1];
					}
					else{
						$previousid = $ids[0];
					}
					if($index < $idCount){
						if(isset($ids[$index + 1])){
							$nextid = $ids[$index + 1];
						}
					}
					else{
						$nextid = $ids[$index];
					}
					break;
				}
				$index++;
			}
		}

		//$_Common->debugPrint("$firstid,$previousid,$nextid,$lastid,$selectedIndex,$idCount");

	return array($firstid,$previousid,$nextid,$lastid,$selectedIndex,$idCount);
	}
	
	// --------------------------------------------------------------
	function setParentCategory(){
		// parent category display in store template title
		$parent = $this->parentCategory;
		$parentNames = explode(":",$this->parentCategory);
		$pCount = count($parentNames);
		if($pCount > 1){
			$parent = $parentNames[$pCount - 2];	
		}
		$parent = ucwords(strtolower(str_replace('_'," ",$parent)));
		$this->parentCategory = $parent;
	}
	
    # -------------------------------------------------------------------
    # Function name
    #   getImages
    # General description
    #   Looks up the images from the images directory
    # Inputs
    #   records
    # Outputs
    #   html for images
    # Modified Date
    #   08/01/03
    #
    function getImages(){

        global $_CF;
        global $_Common;

        $imageURL = "";
		$tWidth = 100;
		$tHeight = 150;
        $fWidth = 100;
        $fHeight = 150;
        $nWidth = 100;
        $nHeight = 150;

		$border = $_CF['images']['image_border_size'];

             # Lookup images from the images directory. Use them if they exist.

        $imagesDir = $_CF['images']['fullsize_images_directory'];

		$thumbsDir = "images/thumbs";
        if(!empty($_CF['images']['thumbnail_images_directory'])){
            $thumbsDir = $_CF['images']['thumbnail_images_directory'];
        } 

        $noImage = empty($_CF['images']['no_photo_image']) ? NULL : $_CF['images']['no_photo_image'];
        if($noImage && !file_exists("$thumbsDir/$noImage")){
            $noImage = null;
        }

        foreach($this->records as $index=>$fields){

			//$_Common->debugPrint($fields);

            $name = trim($fields['name']);
            $display_id = trim($fields['sku']);

            $Thumbnail = null;
            $FullSize = null;

                # Thumbnail

            if(!empty($fields['thumbnail_image'])){
                $thumbName = trim($fields['thumbnail_image']);
                if(file_exists("$thumbsDir/$thumbName")){
					$thumb = "$thumbsDir/$thumbName";
                    $Thumbnail = trim($fields['thumbnail_image']);
                    list($tWidth, $tHeight, $type, $attr) = getimagesize($thumb);
                }				
//                elseif(file_exists("$imagesDir/$thumbName")){
//					$thumb = "$imagesDir/$thumbName";
//                    $Thumbnail = trim($fields['thumbnail_image']);
//                    list($tWidth, $tHeight, $type, $attr) = getimagesize($thumb);
//                }
            }

            if(!$Thumbnail){

                $thumb = "$thumbsDir/$display_id" . ".gif";
                if($Thumbnail == "" && file_exists($thumb)){
                     list($tWidth, $tHeight, $type, $attr) = getimagesize($thumb);
                     $Thumbnail = "$display_id" . ".gif";
                }
                $thumb = "$thumbsDir/$display_id" . ".jpg";
                if($Thumbnail == "" && file_exists($thumb)){
                     list($tWidth, $tHeight, $type, $attr) = getimagesize($thumb);
                     $Thumbnail = "$display_id" . ".jpg";
                }				
                $thumb = "$thumbsDir/$display_id" . "_t.gif";
                if($Thumbnail == "" && file_exists($thumb)){
                     list($tWidth, $tHeight, $type, $attr) = getimagesize($thumb);
                     $Thumbnail = "$display_id" . "_t.gif";
                }
                $thumb = "$thumbsDir/$display_id" . "_t.jpg";
                if($Thumbnail == "" && file_exists($thumb)){
                     list($tWidth, $tHeight, $type, $attr) = getimagesize($thumb);
                     $Thumbnail = "$display_id" . "_t.jpg";
                }
                
                if($Thumbnail == "" && $noImage != ""){
					$Thumbnail = $noImage;
					list($tWidth, $tHeight, $type, $attr) = getimagesize("$thumbsDir/$noImage");
                }
            }

                # Fullsize

            $gotFull = false;
            if(!empty($fields['fullsize_image'])){
                $full = "$imagesDir/" . trim($fields['fullsize_image']);
                if(file_exists($full)){
                    $gotFull = true;
                    list($fWidth, $fHeight, $type, $attr) = getimagesize($full);
                    $FullSize = trim($fields['fullsize_image']);
                }
            }
            if(!$gotFull){
                if(file_exists("$imagesDir/$display_id.gif")){
                     list($fWidth, $fHeight, $type, $attr) = getimagesize("$imagesDir/$display_id.gif");
                     $FullSize = "$display_id.gif";
                }
                if(file_exists("$imagesDir/$display_id.jpg")){
                     list($fWidth, $fHeight, $type, $attr) = getimagesize("$imagesDir/$display_id.jpg");
                     $FullSize = "$display_id.jpg";
                }
            }

            $this->records[$index]['images']['thumbnail']['image'] = $Thumbnail;
            $this->records[$index]['images']['thumbnail']['height'] = $tHeight;
            $this->records[$index]['images']['thumbnail']['width'] = $tWidth;

            $this->records[$index]['images']['fullsize']['image'] = $FullSize;
            $this->records[$index]['images']['fullsize']['height'] = $fHeight;
            $this->records[$index]['images']['fullsize']['width'] = $fWidth;
        }
    }
	

	
    // -------------------------------------------------------------------
    // Description: gets a list of associated options.
    
    function getOptions(){

        global $_STRINGS;
		global $_DB;
        global $_Common;

		// product custom options field
		foreach($this->records as $index=>$data){
			if(!empty($data['options']) && strstr($data['options'],'%%OPTION%%')){
				$this->records[$index]['option_page'] = trim(preg_replace('/%%OPTION%%/i','',$data['options']));
			}
		}

        $optionLabel = $_STRINGS['database_results']['options_label'];

        $sql = "SELECT options.*,option_details.*,product_options.*,products.sku FROM options,option_details,product_options,products
                WHERE options.oid = option_details.oid
                AND product_options.oid = options.oid
                AND products.pid = product_options.pid";
                if(count($this->resultPidList) > 0){
                    $idList = "'" . join("','",$this->resultPidList) . "'";
                    $sql .= " AND product_options.pid IN ($idList)";
                }
                $sql .= " ORDER BY product_options.sequence,option_details.sequence";


			if($this->debug){
				$_Common->debugPrint("<b>Option SQL:</b> $sql");
			}

            $optionRecords = array();
            $options = Array();

			$rs = $_DB->execute($sql);
			if($_DB->numrows($rs) > 0){
				$rows = array();
				while($row = $_DB->fetchrow($rs, "ASSOC")){
					$optionRecords[] = $row;
				}
    			$_DB->free_result($rs);
			}
			$count = 0;
            foreach($optionRecords as $index=>$data){
                $id = $data['pid'];
                $oid = $data['oid'];
                $options[$id][$oid][] = $data;
                $count++;
            }


        if($this->debug){
			$_Common->debugPrint($options);
		}
		
        foreach($this->records as $index=>$data){
            if(isset($data['pid']) && isset($options[$data['pid']])){
                $this->records[$index]['options'] = $options[$data['pid']];
            }
            else{
                $this->records[$index]['options'] = array();
            }
        }
    }
    
	// -------------------------------------------------------------------
    // Description: gets a list of related items.
	function getRelatedItems(){
		
		global $_DB;
        global $_Common;
        
		foreach($this->records as $index=>$fields){
			$pid = $fields['pid'];
			$sql = "SELECT products.sku,products.name,products.price,related_products.related_pid
					FROM products, related_products
					WHERE products.pid = related_products.related_pid 
					AND related_products.pid = '$pid'";
			$related = $_DB->getRecords($sql);
			$this->records[$index]['related_items'] = $related;
			if($this->debug){
				$_Common->debugPrint($related, "Related Items: $sql");
				//exit;
			}
		}
	}

	// -------------------------------------------------------------------
    // Description: gets a list of inventory counts.
	function getInventory(){
		
		global $_Registry;
		global $_Common;
		global $_DB;
		
		$haveInventory = $_Registry->file_exists_incpath("inventory.inc");	

		$pidList = "'" . join("','",$this->resultPidList) . "'";
		$sql = "SELECT inventory.name,inventory.minimum_quantity,inventory.quantity_available,products.sku
				FROM inventory,products
				WHERE products.pid = inventory.pid AND inventory.pid IN ($pidList)";
		$rs = $_DB->execute($sql);
		
		$iList = array();
		
		
		while($row = $_DB->fetchrow($rs, "ASSOC")){
			$sku = $row['sku'];
			unset($row['sku']);
			
			$row['name'] = str_replace("'","`",$row['name']);
			//$row['name'] = str_replace(":","|",$row['name']);
			
			$list = "['" . $row['name'] . "',";
			$list .= "'" . $row['minimum_quantity'] . "',";
			$list .= "'" . $row['quantity_available'] . "'";
			$list .= "]";	
			
			$this->inventory[$sku][] = $list;
			
			$iList[$sku] = array($row['minimum_quantity'],$row['quantity_available']);
			
		}
		// reset the inventory records since multiple categories makes the 
		// inventory count come out wrong. It multiplies by the number of cats
		unset($row);
		foreach($this->records as $i=>$row){
			if(isset($iList[$row['sku']])){
				$this->records[$i]['minimum_quantity'] = $iList[$row['sku']][0];
				$this->records[$i]['available_inventory'] = $iList[$row['sku']][1];
				//print "<!-- sku: " . $iList[$row['sku']][1] . "-->";
			}	
		}
		
		if($this->debug){
			$_Common->debugPrint($this->inventory, "Inventory");
		}
	}

	function isInCart($sku){
	
		global $_CART;
		foreach($_CART as $i=>$row){
			if($row['sku'] == $sku){
				return true;	
			}	
		}	
		return false;		
	}
}
?>











