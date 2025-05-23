<?php
//VersionInfo:Version[3.0.1]
define('ISMENU', true);
global $init;
if(empty($init)){
	// initialize the program and read the config
	include_once("include/initialize.inc");
	$init = new Initialize();
}
global $website_url;

include_once("include/category.menu.inc");
$menu = new Category_menu();
$cats = $menu->getMenuList();
			
//$_Common->debugPrint($cats);
//exit;
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
d.config.inOrder = false;

// root element
d.add(0,-1,'');

<?php foreach($cats as $j=>$row):?>
<?php
	$catid = $row['catid'];
	$parentid = $row['parentid'];
	$catText = $row['category_name'];
	$catLink = addslashes($row['category_name']);
	if(!empty($_REQUEST['catid']) && $row['catid'] == $_REQUEST['catid']){
		$catText = "<span class=dselected>$catText</span>";
	}
	$subCount = 0;
	if(isset($row['subcount'])){
		$subCount = $row['subcount'];
	}
	$count = 0;
	if(isset($row['prodcount'])){
		$count = $row['prodcount'];
	}
	if($subCount == 0 && $count == 0){
		continue;	
	}
	$image = "images/menu/blank.gif";
	if($count > 0){
		$image = "images/menu/arrow_right.gif";
	}
?>
<?php if($count > 0):?>
d.add(<?=$catid;?>, <?=$parentid;?>, '<?=$catText;?> (<?=$count;?>)', '<?=$website_url;?>/products.php?catid=<?=$catid;?>&amp;category=<?=$catLink;?>', '<?=$catText;?>', '', '<?=$image;?>', '<?=$image;?>');
<?php else:?>
d.add(<?=$catid;?>, <?=$parentid;?>, '<?=$catText;?>', '', '<?=$catText;?>', '', '<?=$image;?>', '<?=$image;?>');
<?php endif;?>
<?php endforeach;?>

document.write(d);

<?php if(!empty($_REQUEST['catid']) && in_array($_REQUEST['catid'],array_keys($cats))):?>
d.openTo(<?=$_REQUEST['catid'];?>,false);
<?php endif;?>
//-->
</script>
</div>
<br />