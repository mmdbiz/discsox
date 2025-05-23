<?php
//VersionInfo:Version[3.0.1]

class Menu {

    var $debug = false;
    
    // --------------------------------------------------------------
    function __construct() {

	}

    // --------------------------------------------------------------
    // Displays a list of categories/subcategories and the related product
    // counts from the database
	function getCategories(&$parents,&$subcats){

		global $_DB;
		global $_Common;

        // get parent categories and product counts.
        $sql = "SELECT categories.catid, categories.category_name,categories.category_thumbnail,categories.category_ids,
				COUNT(product_categories.pid) AS count
				FROM categories 
				LEFT JOIN product_categories ON categories.catid = product_categories.catid
				WHERE parentid = '0'
				AND categories.display_category != 'false'
				GROUP BY categories.catid ORDER BY category_name";
		
        $parents = $_DB->getRecords($sql,'catid');

//$_Common->debugPrint($parents);
//exit;

		if(count($parents) > 0){

			$subcats = array();

			foreach($parents as $catid=>$fields){

				$ids = $fields['category_ids'];
				$len = strlen($ids);
				
				$sql = "SELECT categories.catid,parentid,category_name,category_thumbnail,category_link,category_ids,
						COUNT(product_categories.pid) AS count
						FROM categories LEFT JOIN product_categories ON categories.catid = product_categories.catid
						WHERE categories.display_category != 'false'
						AND LEFT(category_ids,$len + 1) = '$ids:'
						GROUP BY categories.catid ORDER BY category_link";

				$records = $_DB->getRecords($sql);

				foreach($records as $i=>$flds){
					
					$cid = $flds['catid'];
					// if the product counts for this category = 0
					// check to make sure we at least have subcats
					if($flds['count'] == 0 && $_DB->getCount('categories',"WHERE categories.parentid = '$cid'") == 0){
						unset($records[$i]);
						continue;	
					}
					else{
						// check product counts with display flag turned off
						// and reset count. If count goes to zero, remove the
						// category from the list.
						$sql = "SELECT COUNT(products.pid) as total
								FROM products,product_categories,manufacturers
								WHERE products.pid = product_categories.pid
								AND manufacturers.mid = products.mid
								AND product_categories.catid = '$cid'
								AND (products.display_product != 'true' OR manufacturers.display_mfg != 'true')";
						
						$count = $_DB->getRecord($sql);
						$records[$i]['count'] -= $count['total'];
						
						if($records[$i]['count'] == 0){
							unset($records[$i]);
							continue;	
						}
					}
				}
				
				$subcats[$catid] = $records;
			}

//$_Common->debugPrint($parents);
//$_Common->debugPrint($subcats);
//exit;
		}
	}
}
global $website_url;
$url = $website_url;
$parents = array();
$subcats = array();
$menu = new Menu();
$menu->getCategories($parents,$subcats);
$idList = array();
$idList = array_keys($parents);
?>

<link rel="StyleSheet" href="styles/dtree.css" type="text/css" />
<script type="text/javascript" src="javascripts/dtree.js"></script>
<div class="dtree">
<script type="text/javascript">
<!--
d = new dTree('d');
d.closeAll();
d.config.useCookies = false;
d.config.useIcons = true;
d.config.useLines = true;
d.config.inOrder = true;

// root element
d.add(0,-1,'');

<?php foreach($parents as $catid=>$row):?>
<?php $pCatName = addslashes($row['category_name']); //$pCatName = ucwords(strtolower(addslashes($row['category_name'])));?>
<?php if($row['count'] > 0):?>
d.add(<?=$catid;?>, 0, '<?=$pCatName;?> (<?=$row['count'];?>)', '<?=$url;?>/products.php?catid=<?=$catid;?>&amp;category=<?=$pCatName;?>', '<?=$pCatName;?>', '', 'images/menu/blank.gif','images/menu/blank.gif');
<?php else:?>
d.add(<?=$catid;?>, 0, '<?=$pCatName;?>', '', '<?=$pCatName;?>','','images/menu/blank.gif','images/menu/blank.gif');
<?php endif;?>
<?php foreach($subcats[$catid] as $i=>$flds):?>
<?php
	$catName = ucwords(strtolower(addslashes($flds['category_name'])));
	$title = $catName; 
	$idList[] = $flds['catid'];
	if(!empty($_REQUEST['catid']) && $flds['catid'] == $_REQUEST['catid']){
		$catName = "<span class=dselected>$catName</span>";
	}
?>
<?php if($flds['count'] > 0):?>
	d.add(<?=$flds['catid'];?>, <?=$flds['parentid'];?>, '<?=$catName;?> (<?=$flds['count'];?>)', '<?=$url;?>/products.php?catid=<?=$flds['catid'];?>&amp;category=<?=$title;?>', '<?=$title;?>', '', 'images/menu/arrow_right.gif','images/menu/arrow_right.gif');
<?php else:?>
	d.add(<?=$flds['catid'];?>, <?=$flds['parentid'];?>, '<?=$catName;?>', '', '<?=$title;?>', '', 'images/menu/blank.gif','images/menu/blank.gif');
<?php endif;?>
<?php endforeach;?>
<?php endforeach;?>
document.write(d);

<?php if(!empty($_REQUEST['catid']) && in_array($_REQUEST['catid'],$idList)):?>
d.openTo(<?=$_REQUEST['catid'];?>,false);
<?php endif;?>
//-->
</script>
</div>
<br clear="all" />




