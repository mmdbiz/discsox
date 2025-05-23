<?php
//VersionInfo:Version[3.0.1]
$_isAdmin = true;
$_adminFunction = "categories";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$data = array();
$parents = array();
$subcats = array();
$edit = false;
$fldProperties = $_DB->getFieldProperties('categories');

$catClass = $_Registry->LoadClass("categories");

	$RUN = false;
	foreach($_REQUEST as $key=>$value){
		switch($key){

			case "add":
			case "edit":
				$catClass->editCategories();
				$edit = true;
				$RUN = 1;
				break;

			case "list":
				$catClass->listCategories();
				$RUN = 1;
				break;

			case "update":
			case "insert":
				$catClass->updateCategories();
				$catClass->editCategories();
				$edit = true;
				$RUN = 1;
				break;

			case "delete":
				$catClass->deleteCategories();
				$catClass->listCategories();
				$RUN = 1;
				break;
		}
		if($RUN){
			break;
		}
	}
	
	if(!$RUN){
		$catClass->listCategories();
	}

?>

<html>
<head>
<title>Category List</title>
<script LANGUAGE="JavaScript">
//<!--
if(eval(parent.menu)) {
	var fileName = parent.menu.location.pathname.substring(parent.menu.location.pathname.lastIndexOf('/')+1);
	if(fileName != "products.menu.html"){
		parent.menu.location = 'menus/products.menu.html';
	}
}
sWidth = screen.width;
var styles = "admin.800.css";
if(sWidth > 850){
    styles = "admin.1024.css";
}
if(sWidth > 1024){
    styles = "admin.1152.css";
}
if(sWidth > 1100){
    styles = "admin.1280.css";
}
document.write('<link rel="stylesheet" href="stylesheets/' + styles + '" type="text/css">');
//-->
</script>
</head>
<body class="mainForm">
<div align=center valign=top style="margin-top:10px;">

<?php if(count($data) == 0):?>

	<p>No categories have been defined in the database.</p>

<?php elseif($edit):?>

	<?php error_reporting(E_PARSE|E_WARNING);?>

	<h4><?=$data['pageTitle'];?></h4>

	<form method="post" action="categories.php" enctype="multipart/form-data">
	<input type=hidden name="catid" value="<?=$data['catid'];?>">

	<table cellpadding=3 cellspacing=2 border=0>
	<tr>
		<td valign=middle align=right>Select Parent Category:</td>
		<td valign=top align=left><?=$data['parentSelectBox'];?></td>
	</tr>
	<tr>
		<td valign=middle align=right>Category Name: </td>
		<td valign=top align=left>
			<input type=text name="category_name" value="<?=stripslashes($data['category_name']);?>" size=50>
		</td>
	</tr>
	<tr>
		<td valign=middle align=right>Display Category: </td>
		<td valign=top align=left>
			<?=$_DB->getDefaultValues('display_category',$data['display_category'],true);?>
		</td>
	</tr>
	<tr>		<td valign=top align=right>Category Page Title:</td>		<td valign=top align=left>			<input type=text name="category_page_title" value="<?=stripslashes($data['category_page_title']);?>" size=50>
		</td>	</tr>
	<tr>		<td valign=top align=right>Category META Description:</td>		<td valign=top align=left>			<textarea name="category_meta_description" rows="5" cols="49" wrap="virtual"><?=stripslashes($data['category_meta_description']);?></textarea>		</td>	</tr>
	<tr>		<td valign=top align=right>Category META Keywords:</td>		<td valign=top align=left>			<textarea name="category_meta_keywords" rows="5" cols="49" wrap="virtual"><?=stripslashes($data['category_meta_keywords']);?></textarea>		</td>	</tr>
	
	<tr>
		<td valign=top align=right>Category Discount:</td>
		<td valign=top align=left>
			<input type=text name="category_discount" value="<?=$data['category_discount'];?>" size=25>
		</td>
	</tr>
	<tr>
		<td valign=top align=right>Apply to Subcategories:</td>
		<td valign=top align=left>
			<?=$_DB->getDefaultValues('apply_to_subcategories',$data['apply_to_subcategories'],true);?>
		</td>
	</tr>
	
	
	<tr>
		<td valign=middle align=right>Category Thumbnail: </td>
		<td valign=top align=left>
			<input type=text name="category_thumbnail" value="<?=$data['category_thumbnail'];?>" size=25 ID="Text1">
		</td>
	</tr>
	<tr>
		<td valign=middle align=right>Upload a Category Image: </td>
		<td valign=top align=left><input type=file name="file-category_image" size=25></td>
	</tr>
	<tr>
		<td valign=middle align=right>Automatically Create Thumbnail: </td>
		<td valign=top align=left>
			<input type=radio name="create_thumbnail" value="true" checked> Yes
			<input type=radio name="create_thumbnail" value="false"> No
		</td>
	</tr>
	</table>

	<p><hr size=1 noshade width=400><br />

	<input class="buttons" type="submit" name="<?=$data['buttonName'];?>" value="<?=$data['buttonText'];?>"><br /><br />

	<?php if($data['buttonName'] == "update"):?>
		<input class="buttons" type="submit" name="delete" value="Delete" onclick="return confirm('Are you sure you want to delete this category?')">&nbsp;
		<input type="checkbox" name="recursive" value="true"> Delete all subcategories and products in this category?
	<?php endif;?>

	</form>



<?php else:?>

	<h4>Category List</h4>

	<table border=0 cellpadding=3 cellspacing=1 align=center width="95%">
		<tr>
			<th align=left>&nbsp;</th>
			<th valign="top">Image</th>
			<th align=left valign="top">Parent Category</th>
			<th align=left valign="top">Sub-Category</th>
			<th align="center" valign="top">Display</th>
			<th align="center" valign="top" nowrap>Products</th>
			<th colspan="2" valign="top">Manage</th>
		</tr>

		<?php
			$counter = 0;
			// row backround colors
			$color = array();
			$color[0] = "#FFFFFF";
			$color[~0] = "#E2EDE2";
			$ck = 0;
			$totalItems = 0;	
		?>
		<?php foreach($parents as $catid=>$row):?>
			<?php
				$totalItems += $row['count'];
				$counter++;
			?>
			<tr bgcolor="<?=$color[$ck = ~$ck];?>">
				<td valign=top align=right width=20><?=trim($counter);?></td>
				<td valign=top width=100><?=$row['category_thumbnail'];?></td>
				<td valign=top width=250 nowrap><?=stripslashes($row['category_name']);?></td>
				<td valign=top>&nbsp;</td>
				<td align=center valign=top><?=$row['display_category'];?></td>
				<td align=right valign=top width=35><?=$row['count'];?></td>
				<td align=center valign=top width=35><a href="categories.php?edit=true&catid=<?=$row['catid'];?>&parid=0">Edit</a></td>
				<td align=center valign=top width=35>
					<?php if($row['count'] > 0):?>
						<a href="products.php?catid=<?=$row['catid'];?>">Items</a>
					<?php else:?>
						Items
					<?php endif;?>
				</td>
			</tr>
			<?php foreach($subcats[$catid] as $i=>$flds):?>
				<?php
					$counter++; 
					$totalItems += $flds['count'];
					$flds['category_link'] = str_replace($row['category_name'] . ':',"",$flds['category_link']);
				?>
				<tr bgcolor="<?=$color[$ck = ~$ck];?>">
					<td valign=top align=right width=20><?=trim($counter);?></td>
					<td valign=top width=100><?=$flds['category_thumbnail'];?></td>
					<td valign=top width=250 nowrap>&nbsp;</td>
					<td valign=top><?=stripslashes($flds['category_link']);?></td>
					<td align=right valign=top width=35><?=$flds['display_category'];?></td>
					<td align=right valign=top width=35><?=$flds['count'];?></td>
					<td align=center valign=top width=35><a href="categories.php?edit=true&catid=<?=$flds['catid'];?>&parid=<?=$flds['parentid'];?>">Edit</a></td>
					<td align=center valign=top width=35>
						<?php if($flds['count'] > 0):?>
							<a href="products.php?catid=<?=$flds['catid'];?>">Items</a>
						<?php else:?>
							Items
						<?php endif;?>
					</td>
				</tr>
			<?php endforeach;?>
			
			
		<?php endforeach;?>
		<tr><td colspan=8>&nbsp;</td></tr>
		<tr><td colspan=8><b>Total items in database = </b><?=$totalItems;?></td></tr>
	</table>

<?php endif;?>

</div>
<p>&nbsp;</p>
</body>
</html>






