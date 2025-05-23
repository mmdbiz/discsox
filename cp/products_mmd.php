<?php
//VersionInfo:Version[3.0.1]
// update

$_isAdmin = true;
$_adminFunction = "products";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

$maxPerScreen = 50;
$hits = 0;
if(!empty($_REQUEST['hits'])){
	$hits = intval($_REQUEST['hits']);	
}

$data = array();
$add = false;
$edit = false;
$vars = array();
$fldProperties = array();

$prodClass = $_Registry->LoadClass("products");

// clear empty files
if(count($_FILES) > 0){
	foreach($_FILES	as $fldname=>$properties){
		if($properties['tmp_name'] == ""){
			unset($_FILES[$fldname]);	
		}
	}
}


	$RUN = false;
	foreach($_REQUEST as $key=>$value){
		switch($key){

			case "add":
			case "edit":
				$vars = $prodClass->editProducts();
				$edit = true;
				$RUN = 1;
				break;

			case "list":
				$vars = $prodClass->listProducts();
				$RUN = 1;
				break;

			case "update":
			case "insert":
				$prodClass->updateProducts();
				$edit = true;
				if(!empty($_REQUEST['show_add'])){
					// Go to add screen
					unset($_REQUEST['pid']);
					$_REQUEST['add'] = "true";
					$vars = $prodClass->editProducts(NULL);
				}
				else{
					// show edit screen
					$pid = $_REQUEST['pid'];
					$vars = $prodClass->editProducts($pid);
				}
				$RUN = 1;
				break;

			case "delete":
				$prodClass->deleteProducts();
				$vars = $prodClass->listProducts();
				$RUN = 1;
				break;
		}
		if($RUN){
			break;
		}
	}
	
	if(!$RUN){
		$vars = $prodClass->listProducts();
	}

if(isset($_REQUEST['add'])){
	$add = true;	
}

// row backround colors
$color = array();
$color[0] = "#FFFFFF";
$color[~0] = "#E2EDE2";
$ck = 0;

$haveInventory = $_Registry->file_exists_incpath("inventory.inc");
$shipExt = $_DB->getRecord("SELECT use_shipping_plugin,shipping_plugin_name FROM shipping");

if(!empty($shipExt['shipping_plugin_name'])){
	$names = explode('.',$shipExt['shipping_plugin_name']);
	$shipExt['shipping_plugin_name'] = strtoupper($names[0]);
	//$_Common->debugPrint($shipExt['shipping_plugin_name']);
}
?>
<html>
<head>
<title>Product List</title>
<script LANGUAGE="JavaScript">
//<!--
if(eval(parent.menu)) {
	var fileName = parent.menu.location.pathname.substring(parent.menu.location.pathname.lastIndexOf('/')+1);
	if(fileName != "products.menu.html"){
		<?php if($haveInventory):?>
		parent.menu.location = 'menus/products.menu.html?true';
		<?php else:?>
		parent.menu.location = 'menus/products.menu.html';
		<?php endif;?>
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

function showIt(whichEl){

    var imageID = "img." + whichEl;

    if(document.all){
        whichEl = document.all[whichEl];
        imageEl = document.all[imageID];
    }
    else{
        whichEl = document.getElementById(whichEl);
        imageEl = document.getElementById(imageID);
    }

    whichEl.style.display = (whichEl.style.display == "none" ) ? "" : "none";

        // Change images

    var imgPath = unescape(imageEl.src).split('/');
    var imgName = imgPath[imgPath.length - 1];
    if(imgName == "plus.gif"){
        imageEl.src = "images/minus.gif";
    }
    else{
        imageEl.src = "images/plus.gif";
    }
}

//-->
function MM_scanStyles(obj, prop) { //v9.0
  var inlineStyle = null; var ccProp = prop; var dash = ccProp.indexOf("-");
  while (dash != -1){ccProp = ccProp.substring(0, dash) + ccProp.substring(dash+1,dash+2).toUpperCase() + ccProp.substring(dash+2); dash = ccProp.indexOf("-");}
  inlineStyle = eval("obj.style." + ccProp);
  if(inlineStyle) return inlineStyle;
  var ss = document.styleSheets;
  for (var x = 0; x < ss.length; x++) { var rules = ss[x].cssRules;
	for (var y = 0; y < rules.length; y++) { var z = rules[y].style;
	  if(z[prop] && (rules[y].selectorText == '*[ID"' + obj.id + '"]' || rules[y].selectorText == '#' + obj.id)) {
        return z[prop];
  }  }  }  return "";
}

function MM_getProp(obj, prop) { //v8.0
  if (!obj) return ("");
  if (prop == "L") return obj.offsetLeft;
  else if (prop == "T") return obj.offsetTop;
  else if (prop == "W") return obj.offsetWidth;
  else if (prop == "H") return obj.offsetHeight;
  else {
    if (typeof(window.getComputedStyle) == "undefined") {
	    if (typeof(obj.currentStyle) == "undefined"){
		    if (prop == "P") return MM_scanStyles(obj,"position");
        else if (prop == "Z") return MM_scanStyles(obj,"z-index");
        else if (prop == "V") return MM_scanStyles(obj,"visibility");
	    } else {
	      if (prop == "P") return obj.currentStyle.position;
        else if (prop == "Z") return obj.currentStyle.zIndex;
        else if (prop == "V") return obj.currentStyle.visibility;
	    }
    } else {
	    if (prop == "P") return window.getComputedStyle(obj,null).getPropertyValue("position");
      else if (prop == "Z") return window.getComputedStyle(obj,null).getPropertyValue("z-index");
      else if (prop == "V") return window.getComputedStyle(obj,null).getPropertyValue("visibility");
    }
  }
}

function MM_dragLayer(objId,x,hL,hT,hW,hH,toFront,dropBack,cU,cD,cL,cR,targL,targT,tol,dropJS,et,dragJS) { //v9.01
  //Copyright 2005-2006 Adobe Macromedia Software LLC and its licensors. All rights reserved.
  var i,j,aLayer,retVal,curDrag=null,curLeft,curTop,IE=document.all;
  var NS=(!IE&&document.getElementById); if (!IE && !NS) return false;
  retVal = true; if(IE && event) event.returnValue = true;
  if (MM_dragLayer.arguments.length > 1) {
    curDrag = document.getElementById(objId); if (!curDrag) return false;
    if (!document.allLayers) { document.allLayers = new Array();
      with (document){ if (NS) { var spns = getElementsByTagName("span"); var all = getElementsByTagName("div");
        for (i=0;i<spns.length;i++) if (MM_getProp(spns[i],'P')) allLayers[allLayers.length]=spns[i];}
        for (i=0;i<all.length;i++) {
	        if (MM_getProp(all[i],'P')) allLayers[allLayers.length]=all[i]; 
        }
    } }
    curDrag.MM_dragOk=true; curDrag.MM_targL=targL; curDrag.MM_targT=targT;
    curDrag.MM_tol=Math.pow(tol,2); curDrag.MM_hLeft=hL; curDrag.MM_hTop=hT;
    curDrag.MM_hWidth=hW; curDrag.MM_hHeight=hH; curDrag.MM_toFront=toFront;
    curDrag.MM_dropBack=dropBack; curDrag.MM_dropJS=dropJS;
    curDrag.MM_everyTime=et; curDrag.MM_dragJS=dragJS;
  
    curDrag.MM_oldZ = MM_getProp(curDrag,'Z');
    curLeft = MM_getProp(curDrag,'L');
    if (String(curLeft)=="NaN") curLeft=0; curDrag.MM_startL = curLeft;
    curTop = MM_getProp(curDrag,'T');
    if (String(curTop)=="NaN") curTop=0; curDrag.MM_startT = curTop;
    curDrag.MM_bL=(cL<0)?null:curLeft-cL; curDrag.MM_bT=(cU<0)?null:curTop-cU;
    curDrag.MM_bR=(cR<0)?null:curLeft+cR; curDrag.MM_bB=(cD<0)?null:curTop+cD;
    curDrag.MM_LEFTRIGHT=0; curDrag.MM_UPDOWN=0; curDrag.MM_SNAPPED=false; //use in your JS!
    document.onmousedown = MM_dragLayer; document.onmouseup = MM_dragLayer;
    if (NS) document.captureEvents(Event.MOUSEDOWN|Event.MOUSEUP);
    } else {
    var theEvent = ((NS)?objId.type:event.type);
    if (theEvent == 'mousedown') {
      var mouseX = (NS)?objId.pageX : event.clientX + document.body.scrollLeft;
      var mouseY = (NS)?objId.pageY : event.clientY + document.body.scrollTop;
      var maxDragZ=null; document.MM_maxZ = 0;
      for (i=0; i<document.allLayers.length; i++) { aLayer = document.allLayers[i];
        var aLayerZ = MM_getProp(aLayer,'Z');
        if (aLayerZ > document.MM_maxZ) document.MM_maxZ = aLayerZ;
        var isVisible = (MM_getProp(aLayer,'V')).indexOf('hid') == -1;
        if (aLayer.MM_dragOk != null && isVisible) with (aLayer) {
          var parentL=0; var parentT=0;
          if (NS) { parentLayer = aLayer.parentNode;
            while (parentLayer != null && parentLayer != document && MM_getProp(parentLayer,'P')) {
              parentL += parseInt(MM_getProp(parentLayer,'L')); parentT += parseInt(MM_getProp(parentLayer,'T'));
              parentLayer = parentLayer.parentNode;
              if (parentLayer==document) parentLayer = null;
          } } else if (IE) { parentLayer = aLayer.parentElement;       
            while (parentLayer != null && MM_getProp(parentLayer,'P')) {
              parentL += MM_getProp(parentLayer,'L'); parentT += MM_getProp(parentLayer,'T');
              parentLayer = parentLayer.parentElement; } }
          var tmpX=mouseX-((MM_getProp(aLayer,'L'))+parentL+MM_hLeft);
          var tmpY=mouseY-((MM_getProp(aLayer,'T'))+parentT+MM_hTop);
          if (String(tmpX)=="NaN") tmpX=0; if (String(tmpY)=="NaN") tmpY=0;
          var tmpW = MM_hWidth;  if (tmpW <= 0) tmpW += MM_getProp(aLayer,'W');
          var tmpH = MM_hHeight; if (tmpH <= 0) tmpH += MM_getProp(aLayer,'H');
          if ((0 <= tmpX && tmpX < tmpW && 0 <= tmpY && tmpY < tmpH) && (maxDragZ == null
              || maxDragZ <= aLayerZ)) { curDrag = aLayer; maxDragZ = aLayerZ; } } }
      if (curDrag) {
        document.onmousemove = MM_dragLayer;
        curLeft = MM_getProp(curDrag,'L');
        curTop = MM_getProp(curDrag,'T');
        if (String(curLeft)=="NaN") curLeft=0; if (String(curTop)=="NaN") curTop=0;
        MM_oldX = mouseX - curLeft; MM_oldY = mouseY - curTop;
        document.MM_curDrag = curDrag;  curDrag.MM_SNAPPED=false;
        if(curDrag.MM_toFront) {
          var newZ = parseInt(document.MM_maxZ)+1;
          eval('curDrag.'+('style.')+'zIndex=newZ');
          if (!curDrag.MM_dropBack) document.MM_maxZ++; }
        retVal = false; if(!NS) event.returnValue = false;
    } } else if (theEvent == 'mousemove') {
      if (document.MM_curDrag) with (document.MM_curDrag) {
        var mouseX = (NS)?objId.pageX : event.clientX + document.body.scrollLeft;
        var mouseY = (NS)?objId.pageY : event.clientY + document.body.scrollTop;
        var newLeft = mouseX-MM_oldX; var newTop  = mouseY-MM_oldY;
        if (MM_bL!=null) newLeft = Math.max(newLeft,MM_bL);
        if (MM_bR!=null) newLeft = Math.min(newLeft,MM_bR);
        if (MM_bT!=null) newTop  = Math.max(newTop ,MM_bT);
        if (MM_bB!=null) newTop  = Math.min(newTop ,MM_bB);
        MM_LEFTRIGHT = newLeft-MM_startL; MM_UPDOWN = newTop-MM_startT;
        if (NS){style.left = newLeft + "px"; style.top = newTop + "px";}
        else {style.pixelLeft = newLeft; style.pixelTop = newTop;}
        if (MM_dragJS) eval(MM_dragJS);
        retVal = false; if(!NS) event.returnValue = false;
    } } else if (theEvent == 'mouseup') {
      document.onmousemove = null;
      if (NS) document.releaseEvents(Event.MOUSEMOVE);
      if (NS) document.captureEvents(Event.MOUSEDOWN); //for mac NS
      if (document.MM_curDrag) with (document.MM_curDrag) {
        if (typeof MM_targL =='number' && typeof MM_targT == 'number' &&
            (Math.pow(MM_targL-(MM_getProp(document.MM_curDrag,'L')),2)+
             Math.pow(MM_targT-(MM_getProp(document.MM_curDrag,'T')),2))<=MM_tol) {
          if (NS) {style.left = MM_targL + "px"; style.top = MM_targT + "px";}
          else {style.pixelLeft = MM_targL; style.pixelTop = MM_targT;}
          MM_SNAPPED = true; MM_LEFTRIGHT = MM_startL-MM_targL; MM_UPDOWN = MM_startT-MM_targT; }
        if (MM_everyTime || MM_SNAPPED) eval(MM_dropJS);
        if(MM_dropBack) {style.zIndex = MM_oldZ;}
        retVal = false; if(!NS) event.returnValue = false; }
      document.MM_curDrag = null;
    }
    if (NS) document.routeEvent(objId);
  } return retVal;
}
</script>
<script LANGUAGE="JavaScript" src="javascripts/products.js"></script>
<link rel="stylesheet" href="../../styles/items.css" type="text/css"/>
<!--<link rel="stylesheet" href="../../styles/mmd.css" type="text/css"/>-->
<link rel="stylesheet" href="../../styles/products.css" type="text/css"/>
<link rel="stylesheet" href="../../styles/mainlayout_new.css" type="text/css" />


<style type="text/css">
<!--
#apDiv1 {
	position:absolute;
	width:383px;
	height:223px;
	z-index:1;
	left: 592px;
	top: 216px;
}
:target
	{
	background-color: #FFFF00;
	}
	
.red
	{
	background-color: #FFFF00;
	color: #FF0000;
	}	
-->
</style>
</head>
<body class="mainForm" onLoad="MM_dragLayer('apDiv1','',0,0,0,0,true,false,-1,-1,-1,-1,false,false,0,'',false,'')">
<div align=center valign=top style="margin-top:10px;">

<?php if(count($data) == 0):?>

	<p>No products have been defined in the database.</p>

<?php elseif($edit):?>

	<?php error_reporting(E_PARSE|E_WARNING);?>

	<form id="frmMain" method=post action="products.php" enctype="multipart/form-data">
		
		<?php if(!empty($_REQUEST['add'])):?>
			<h4>Add New Product</h4>
			<input type=hidden name="insert" value="true">
		<?php else:?>
			<h4 style="line-height:10px; margin:10px">Editing Item <?=$data['sku'];?> (<?=$data['name'];?>)</h4>
			<?php if($vars['navLinks']):?>
				<p><?=$vars['navLinks'];?></p>
			<?php endif;?>
			<input type=hidden name="pid" value="<?=$data['pid'];?>" ID="Hidden1">
			<input class=buttons type="submit" name="update" value="Update Product" onClick="return testEntries(this.form);"> &nbsp; 
			<input class=buttons type="submit" name="delete" value="Delete Product" onClick="return confirm('Are you sure you want to delete this product?')"><br />
			<div id="apDiv1">
			  <div class="SummayWrapper" style="text-align:left">
<a name="dvdPro" id="dvdPro"></a><div  id="ItemWrapper" >
  <div id="ItemImage" ><a href="#" id="thumb"><img src="../../graphics/products/dvd-pro_xsmall.gif" alt="DVD Pro Sleeve" width="64" height="80" border="0" /></a></div>
  <div id="ItemFeatures">
    <div id="ItemTitlePop"><a href="#" class="txt123" onClick="MM_openBrWindow('../../products/dvd-pro-sleeve-tab.htm','','scrollbars=yes,resizable=yes,width=770,height=500')">DVD Pro Sleeve - 25 Pack</a><br />
    </div>
    <div id="ItemTitleLink"><a href="#" class="txt123" id="sku_name">DVD Pro Sleeve - 25 Pack</a><br />
    </div>
    <ul>
      <li><a href="#"  id="description">Holds&nbsp;all&nbsp;parts&nbsp;of&nbsp;original&nbsp;DVD: movie&nbsp;poster,&nbsp;DVD wrap&nbsp;and&nbsp;2 DVDs</a></li>
    </ul>
  </div>
  <div id="ItemSeparator">
    <!-- x -->
  </div>
  <div id="Patent"><a href="#" id="patent">Patent No 7,320,400</a></div>
              <div id="ItemPriceTitle"><a href="#" id="retail_price_title">Our Price:</a></div>
  <div id="ItemVolDiscountMain" ><a href="#" id="SleeveDPMain" >Volume Discounts</a>
    <script type="text/javascript">
		   var hb1 = new HelpBalloon({ 
				title: 'Volume Pricing', 
				dataURL: 'tooltips/pricingdvdpro.htm', 
				icon: $('SleeveDPMain'), 
				useEvent: ['mouseover'] 
			}); 
                  </script>
  </div>
  <div id="ItemVolDiscountProd" ><a href="products/dvd-pro-sleeve.htm" id="SleeveDPAcc" >Volume Discounts</a>
    <script type="text/javascript">
		   var hb1 = new HelpBalloon({ 
				title: 'Volume Pricing', 
				dataURL: '../tooltips/pricingdvdpro.htm', 
				icon: $('SleeveDPAcc'), 
				useEvent: ['mouseover'] 
			}); 
                  </script>
  </div>
  <div id="ItemPrice1">
    <div id="ItemMoreInfo"><a href="#" id="MoreInfo">More Info...</a></div>
      <strong><a href="#" id="retail_price">$26.95</a></strong> -  Qty:
    <input type="text" size="1" name="item-25PDP|1:26.95,20:25.25,40:23.35,100:22.05|DiscSox DVD Pro Sleeves 25-Pack|In Stock|1.7" value="0" align="middle" />
  </div>
  <div class="ItemsAddToCart">
    <input name="add_to_cart" type="image" value="submit" src="../../graphics/ybuttons/add_to_cart.gif" alt="Add to cart" />
  </div>
</div>
<!-- #EndLibraryItem --></div></div>
<br />
			
		<?php endif;?>

		<table border="1" cellpadding="3" cellspacing="0" ID="Table1" style="border-collapse:collapse;" width="700">

			<tr>
				<td width="10" valign="top"><img id="img.general" name="img.general" src="images/minus.gif" style="cursor:hand;" onClick="showIt('general');"></td>
		  		<td width="100%" align="left"><span style="cursor:hand;color:#800000" onClick="showIt('general');"><b>General Product Information</b></span></td>
			</tr>

			<tr id="general" style="display:;">
				<td colspan="2" align="center" valign="top">
					<table border="0" cellpadding="3" cellspacing="1" width="100%">
						<tr>
		  					<td align=right valign=top><strong class="red">Select Category:</strong></td>
<td align=left valign=top><b>NOTE:</b> Hold CTRL key and click to select more than one category<br><br>
								<?=$vars['parentSelectBox'];?>							</td>
						</tr>
						<tr>
		  					<td align=right valign=top>Manufacturer:</td>
							<td align=left valign=top>
								<?=$mfgSelect;?>							</td>
						</tr>
	  					<?php if($vars['optionCount'] > 0 && empty($_REQUEST['add'])):?>
						<tr>
    	  					<td align=right valign=top><strong class="red">Product Options</strong>:<br>
   	  					    (Kits only!)</td>
<td align=left valign=top>
								<input type=button value="Select Options" onClick="return OptionListWindow('<?=$data['pid'];?>','<?=$data['name'];?>','<?=session_id();?>')" ID="Button1" NAME="Button1">
								<?php if($vars['product_option_count'] > 0):?>
									&nbsp; (<?=$vars['product_option_count'];?> Options Selected)
								<?php endif;?>							</td>
						</tr>
						<?php endif;?>
						<?php if(empty($_REQUEST['add'])):?>
						<tr>
    	  					<td align=right valign=top>Related Products:</td>
<td align=left valign=top>
								<input type=button value="Select Products" onClick="return RelatedListWindow('<?=$data['pid'];?>','<?=$data['name'];?>','<?=session_id();?>')" ID="Button2" NAME="Button2">							</td>
						</tr>
						<?php endif;?>	
						<tr>
		  					<td align=right valign=top><strong class="red">Display Product:</strong></td>
<td align=left valign=top>
								<?=$data['display_product'];?>							</td>
						</tr>
						<tr>
		  					<td align=right valign=top><strong class="red">SKU:</strong></td>
<td align=left valign=top>
		    					<input type=text name="sku" value="<?=$data['sku'];?>" size=60 ID="Text1">							</td>
						</tr>
						<tr>
		  					<td align=right valign=top><a href="#sku_name" class="style1"><strong class="red">Name</strong></a>:</td>
<td align=left valign=top>
		    					<input type=text name="name" value="<?=str_replace("\""," Inch",stripslashes($data['name']));?>" size=60 ID="Text2">							</td>
						</tr>
						<tr>
						  <td align=right valign=top><strong><a href="#retail_price_title" class="red">Retail Price Title</a>:</strong></td>
						  <td align=left valign=top><input type=text name="retail_price_title" value="<?=$data['retail_price_title'];?>" size=60 ID="Text3">							</td>
					  </tr>
						<tr>
		  					<td align=right valign=top><strong><a href="#retail_price" class="red">Retail Price</a>:</strong></td>
<td align=left valign=top>
								<input type=text name="retail_price" value="<?=$data['retail_price'];?>" size=60 ID="Text3">							</td>
						</tr>
						<tr>
		  					<td align=right valign=top><strong><a href="#SleeveDPMain" class="red">Website Price</a>:</strong></td>
<td align=left valign=top>
		    					<input type=text name="price" value="<?=$data['price'];?>" size=60 ID="Text3">
		    					[<a href="#" onClick="return OpenQtyWindow('price')">Qty Price</a>]							</td>
						</tr>	
						<tr>
		  					<td align=right valign=top><strong class="red">On Sale:</strong></td>
<td align=left valign=top>
								<?=$data['on_sale'];?>							</td>
						</tr>
						<tr>
		  					<td align=right valign=top><strong class="red">Sale Price:</strong></td>
<td align=left valign=top>
		    					<input type=text name="sale_price" value="<?=$data['sale_price'];?>" size=60 ID="Text3">
		    					[<a href="#" onClick="return OpenQtyWindow('sale_price')">Sale Qty Price</a>]							</td>
						</tr>
						<tr>
		  					<td align=right valign=top><a href="#description" class="red"><strong>Description</strong></a>:</td>
<td align=left valign=top>
		    					<textarea name="description" rows=10 cols=59 wrap=virtual ID="Textarea1"><?=stripslashes($data['description']);?></textarea>							</td>
						</tr>
						<tr>
		  					<td align=right valign=top><strong class="red"># of Items in Product:</strong><br>
(Sleeves only)</td>
<td align=left valign=top>
		    					<input type=text name="size" value="<?=$data['size'];?>" size=60 ID="Text5">							</td>
						</tr>
						<tr>
		  					<td align=right valign=top><strong class="red">Weight:</strong></td>
<td align=left valign=top>
		    					<input type=text name="weight" value="<?=$data['weight'];?>" size=60 ID="Text6">
		    					<?php if($shipExt['use_shipping_plugin'] == 'true' && trim($shipExt['shipping_plugin_name']) != ""):?>
									<br />[<a href="#" onClick="return shipDefineWindow('weight','<?=$shipExt['shipping_plugin_name'];?>')">Define <?=$shipExt['shipping_plugin_name'];?> Package</a>]
								<?php endif;?>							</td>
						</tr>
						<tr>
		  					<td align=right valign=top><a href="#MoreInfo"><strong class="red">Link Page</strong></a>:</td>
<td align=left valign=top>
		    					<input type=text name="link_page" value="<?=$data['link_page'];?>" size=60 ID="Text15">							</td>
						</tr>
						<tr>
		  					<td align=right valign=top><a href="#MoreInfo"><strong class="red">Link Text:</strong></a></td>
<td align=left valign=top>
		    					<input type=text name="link_text" value="<?=$data['link_text'];?>" size=60 ID="Text16">							</td>
						</tr>
						<tr>
		  					<td align=right valign=top>Is Taxable:</td>
							<td align=left valign=top>
								<?=$data['is_taxable'];?>							</td>
						</tr>
						<tr>
		  					<td align=right valign=top>Tax Rate:</td>
							<td align=left valign=top>
		    					<input class=rightalign type=text name="tax_rate" value="<?=$data['tax_rate'];?>" size="5" ID="Text7">							</td>
						</tr>
						<tr>
		  					<td align=right valign=top>Is Downloadable:</td>
							<td align=left valign=top>
								<?=$data['is_downloadable'];?>							</td>
						</tr>
						<tr>
		  					<td align=right valign=top><strong class="red">WS Margin Adjust:</strong></td>
							<td align=left valign=top>
		    					<input type="text" name="download_filename" value="<?=$data['download_filename'];?>" size=60 ID="Text7">							</td>
						</tr>	
						<tr>
		  					<td align=right valign=top>Last Modified Date:</td>
							<td align=left valign=top>
								<?php if($data['last_modified'] != "" && $data['last_modified'] != "0000-00-00"):?>
    		    					<?=$data['last_modified'];?>
								<?php else:?>
    		    					<?=date("Y-m-d");?>
								<?php endif;?>							</td>
						</tr>
						
						<?php if(isset($data['detail_page_title'])):?>
							<tr>
		  						<td align=right valign=top><strong class="red">Detail Page Title:</strong></td>
<td align=left valign=top>
		    						<input type="text" name="detail_page_title" value="<?=str_replace("\""," Inch",$data['detail_page_title']);?>" size=60 ID="Text7">								</td>
							</tr>
						<?php endif;?>
						
						<?php if(isset($data['detail_meta_description'])):?>
							<tr>
		  						<td align=right valign=top nowrap>Detail Meta Description:</td>
								<td align=left valign=top>
		    						<textarea name="detail_meta_description" rows=5 cols=59 wrap=virtual ID="Textarea1"><?=stripslashes($data['detail_meta_description']);?></textarea>								</td>
							</tr>
						<?php endif;?>
						
						<?php if(isset($data['detail_meta_keywords'])):?>
							<tr>
		  						<td align=right valign=top>Detail Meta Keywords:</td>
								<td align=left valign=top>
		    						<textarea name="detail_meta_keywords" rows=5 cols=59 wrap=virtual ID="Textarea1"><?=stripslashes($data['detail_meta_keywords']);?></textarea>								</td>
							</tr>
						<?php endif;?>
						
						
						<tr><td colspan="2">&nbsp;</td></tr>
					</table>
			  </td>
			</tr>

			<tr>
				<td width="10" valign="top"><img id="Img3" name="img.images" src="images/plus.gif" style="cursor:hand;" onClick="showIt('images');"></td>
		  		<td width="100%" align="left"><span style="cursor:hand;color=#800000;" onClick="showIt('images');"><b>Images</b></span></td>
			</tr>

			<tr id="images" style="display:none;">
			
				<td>&nbsp;</td>
				<td align="center" valign="top" style="border:1px solid #336699;">
				
	  				<table border="0" cellpadding=3 cellspacing=1 ID="Table4">
	  					<tr><td colspan=2>&nbsp;</td></tr>
						<tr>
		  					<td align=right valign=top><a href="#thumb"><strong class="red">Thumbnail Image</strong></a>:</td>
<td align=left valign=top>
		    					<input type=text name="thumbnail_image" value="<?=$data['thumbnail_image'];?>" size=40 ID="Text12">
							</td>
						</tr>
						<tr>
		  					<td align=right valign=top>Upload thumbnail_image:</td>
							<td>
								<input type="file" name="file-thumbnail_image" size=50 ID="File1">
							</td>
						</tr>
						<tr>
		  					<td align=right valign=top><strong class="red">Fullsize Image</strong>:</td>
<td align=left valign=top>
		    					<input type=text name="fullsize_image" value="<?=$data['fullsize_image'];?>" size=40 ID="Text13">
							</td>
						</tr>
						<tr>
		  					<td align=right valign=top>Upload fullsize_image:</td>
							<td>
								<input type="file" name="file-fullsize_image" size=50 ID="File2">
							</td>
						</tr>
						<tr><td colspan=2>&nbsp;</td></tr>
						<tr>
		  					<td colspan=2 align=center>
		  					Automatically resize Fullsize image upload to <?=$_CF['images']['product_thumbnail_max_height'];?> high
							X <?=$_CF['images']['product_thumbnail_max_width'];?> wide and save as thumbnail?	
		  					</td>
						</tr>
						<tr>
		  					<td align=right valign=middle>Create Thumbnail:</td>
							<td>
								<input type=radio name="create_thumbnail" value="true" ID="Radio1"> Yes
								<input type=radio name="create_thumbnail" value="false" checked ID="Radio2"> No &nbsp;
							</td>
						</tr>
						<tr><td colspan=2>&nbsp;</td></tr>
	  				</table>

				</td>
			</tr>

			<?php if($vars['haveInventoryClass']):?>

				<tr>
					<td width="10" valign="top"><img id="Img6" name="img.inventory" src="images/plus.gif" style="cursor:hand;" onClick="showIt('inventory');"></td>
		  			<td width="100%" align="left"><span style="cursor:hand;color:#800000" onClick="showIt('inventory');"><b>Inventory for: <?=$data['name'];?></b></span></td>
				</tr>

				<tr id="inventory" style="display:none;">
				
					<td>&nbsp;</td>
					<td align="left" valign="top" style="border:1px solid #336699; padding-left:10px;">

						<p>You must set the item as an inventory item and save it before you can set the inventory values.</p>
						
						<table border=0 cellpadding=3 cellspacing=1 ID="Table7" align="center" width="98%">
							<tr>
		  						<td align=right valign="top" width="50%">Inventory Item:</td>
								<td align=left valign="top"  width="50%">
									<?=$data['inventory_item'];?>
								</td>
							</tr>
							<tr>
		  						<td align=right valign=top>Inventory Options:</td>
								<td align=left valign=top>
									<?=$data['inventory_options'];?>
								</td>
							</tr>
							<tr>
		  						<td align=right valign=top>Display when sold out:</td>
								<td align=left valign=top>
									<?=$data['display_when_sold_out'];?>
								</td>
							</tr>
							
							<?php if($vars['isInventoried']): ?>
							
								<?php if($vars['optionsInventoried'] && count($vars['inventoryData']) > 0):?>

 									<tr>
 		  								<td valign="top" colspan="2">
	 		  							
 		  									<table border="1" cellpadding="3" cellspacing="0" width="100%">
 		  										<tr>
 		  											<th>Option(s)</th>
 		  											<th nowrap>Min Qty</th>
 		  											<!--th>Max Qty</th-->
 		  											<th>Available</th>
 		  											<th>Sold</th>
 		  										</tr>
 		  										<?php foreach($vars['inventoryData'] as $i=>$flds):?>
 		  										
 		  										<tr>
 		  											<td width="70%">
 		  												<?php
 		  													$flds['name'] = str_replace("<br>","",$flds['name']);
 		  													$flds['name'] = str_replace('"',"&quot;",$flds['name']);
 		  													// this is for only one option in inventory. Was updated
 		  													// in inventory.inc line 88 - 10/24/07
 		  													if(empty($flds['oids']) && !empty($flds['oid'])){
 		  														$flds['oids'] = $flds['oid'];
 		  													}
 		  													if(empty($flds['odids']) && !empty($flds['odid'])){
 		  														$flds['odids'] = $flds['odid'];
 		  													}
 		  												?>
 		  												<input type="hidden" name="inventory[<?=$i;?>][pid]" value="<?=$flds['pid'];?>">
 		  												<input type="hidden" name="inventory[<?=$i;?>][oids]" value="<?=$flds['oids'];?>">
 		  												<input type="hidden" name="inventory[<?=$i;?>][odids]" value="<?=$flds['odids'];?>">
 		  												<input type="hidden" name="inventory[<?=$i;?>][name]" value="<?=$flds['name'];?>">
 		  												<?=$flds['name'];?>
 		  											</td>
 		  											<td>
 		  												<input class="rightalign" type="text" name="inventory[<?=$i;?>][minimum_quantity]" value="<?=$flds['minimum_quantity'];?>" size="5">
 		  											</td>
 		  											<!--td>
 		  												<input class="rightalign" type="text" name="inventory[<?=$i;?>][maximum_quantity]" value="<?=$flds['maximum_quantity'];?>" size="10">
 		  											</td-->
 		  											<td align="center">
 		  												<input class="rightalign" type="text" name="inventory[<?=$i;?>][quantity_available]" value="<?=$flds['quantity_available'];?>" size="5">
 		  											</td>
 		  											<td>
 		  												<input class="rightalign" type="text" name="inventory[<?=$i;?>][quantity_sold]" value="<?=$flds['quantity_sold'];?>" size="5">
 		  											</td>
 		  										</tr>
 		  										<?php endforeach;?>
 		  									</table>
 		  								</td>
 									</tr>
								
								<?php else:?>		  		
 									<tr>
 		  								<td align=right valign=top>Minimum Quantity:</td>
 										<td align=left valign=top>
 											<input type="hidden" name="inventory[0][pid]" value="<?=$vars['pid'];?>">
 											<input type="hidden" name="inventory[0][name]" value="<?=$data['name'];?>">
 		    								<input class=rightalign type=text name="inventory[0][minimum_quantity]" value="<?=$vars['inventoryData'][0]['minimum_quantity'];?>" size=8 ID="Text24">
 										</td>
 									</tr>
 									<!--tr>
 		  								<td align=right valign=top>Maximum Quantity Per Order:</td>
 										<td align=left valign=top>
 		    								<input class=rightalign type=text name="inventory[0][maximum_quantity]" value="<?=$vars['inventoryData'][0]['maximum_quantity'];?>" size=8 ID="Text25">
 										</td>
 									</tr-->
 									<tr>
 		  								<td align=right valign=top>Quantity Available:</td>
 										<td align=left valign=top>
 		    								<input class=rightalign type=text name="inventory[0][quantity_available]" value="<?=$vars['inventoryData'][0]['quantity_available'];?>" size=8 ID="Text26">
 										</td>
 									</tr>
 									<tr>
 		  								<td align=right valign=top>Quantity Sold:</td>
 										<td align=left valign=top>
 		    								<input class=rightalign type=text name="inventory[0][quantity_sold]" value="<?=$vars['inventoryData'][0]['quantity_sold'];?>" size=8 ID="Text27">
 										</td>
 									</tr>
 								<?php endif;?>
	 							
							<?php endif; ?>
							
							<tr><td colspan=2>&nbsp;</td></tr>
	  					</table>
					</td>
				</tr>
			<?php endif;?>

			<tr>
				<td width="10" valign="top"><img id="Img7" name="img.custom" src="images/plus.gif" style="cursor:hand;" onClick="showIt('custom');"></td>
		  		<td width="100%" align="left"><span style="cursor:hand;color=#800000;" onClick="showIt('custom');"><b>Custom Fields</b></span></td>
			</tr>

			<tr id="custom" style="display:none;">
				<td>&nbsp;</td>
				<td align="center" valign="top" style="border:1px solid #336699;">

					<table border=0 cellpadding=3 cellspacing=1 ID="Table8">
						<tr><td colspan=2>&nbsp;</td></tr>

						<?php foreach($data as $key=>$value):?>
							<?php 
								if(!isset($vars['customFields'][$key])){
									continue;
								}
								if(substr($key,0,7) == "detail_"){
									continue;	
								}
							?>
							<?php
								$strLen = 50;
								$match = array();
								preg_match("|varchar\((.*)\)|",$fldProperties[$key][1],$match);
								if(isset($match[1]) && $match[1] > 50 && strLen($value) > 50){
									$strLen = $match[1];
								}
								elseif($fldProperties[$key][1] == "text"){
									$strLen = 255;
								}
							?>
						
							<?php if(stristr($key,"id")){continue;}?>
							<?php $displayKey = ucwords(preg_replace("|\_|"," ",$key));?>
							<tr>
								<td align="right" valign="top"><a href="#<?=$displayKey;?>">
							    <strong class="red"><?=$displayKey;?></strong>
							    : </a></td>
				  <td valign="top">
									<?php if(substr($value,0,7) != "<select"):?>
									
										<?php if($strLen > 50):?>
											<textarea name="<?=$key;?>" rows="10" cols="50" wrap="virtual" ID="Textarea2"><?=$value;?></textarea>
										<?php else:?>
											<input type="text" name="<?=$key;?>" value="<?=$value;?>" size="<?=$strLen;?>" ID="Text28">
										<?php endif;?>
										
									<?php else:?>
									
										<?=$value;?>
									
									<?php endif;?>								</td>
							</tr>
						<?php endforeach;?>	
						
						<tr><td colspan=2>&nbsp;</td></tr>
					</table>
					
			  </td>
			</tr>
			
		</table>

		<p align="center">
			<?php if($add):?>
				<input class=buttons type="submit" name="insert" value="Add Product" onClick="return testEntries(this.form);">
				<?php if($_REQUEST['show_add'] == "true"):?>
					&nbsp;	<input type="checkbox" name="show_add" value="true" checked> Keep showing add screen
				<?php else:?>
					&nbsp;	<input type="checkbox" name="show_add" value="true"> Keep showing add screen
				<?php endif;?>	
			<?php else:?>
				<input class=buttons type="submit" name="update" value="Update Product" onClick="return testEntries(this.form);"> &nbsp; 
				<input class=buttons type="submit" name="delete" value="Delete Product" onClick="return confirm('Are you sure you want to delete this product?')">
			<?php endif;?>
		</p>

	</form>

	<script type="text/javascript">
		var selbox = "catid[]";
		var sIndex = document.forms['frmMain'].elements[selbox].selectedIndex;
		if(sIndex > 4){
			setTimeout("document.forms['frmMain'].elements[selbox].options[sIndex].selected=true;",10);
		}
	</script>


<?php else:?>

	<form method="get" action="products.php">
		<table border="0" cellpadding="3" cellspacing="0" align="center" width="95%" ID="Table2" style="border:1px solid #E5E5E5;">
			<tr bgcolor="#e2eDe2">
				<td>
					<b>Select Category:</b> <?=$vars['categoryList'];?>
				</td>
				<td align="right">
					<b>Enter SKU:</b> <input type="text" name="sku" size="10"> <input type="submit" name="go" value="Search">
				</td>
			</tr>
			<tr><td colspan="2" style="line-height:2px;">&nbsp;</td></tr>
			<tr>
				<td colspan="2" align="center">
					<?php if($vars['catid']):?>
						<h4><?=$vars['start'];?> - <?=$vars['end'];?> of <?=$vars['rsCount'];?> Items
					<?php else:?>
						<h4><?=$vars['start'];?> - <?=$vars['end'];?> of <?=$vars['rsCount'];?> Items
					<?php endif;?>
				</td>
			</tr>
			<tr>
				<td><b>Click on field name to sort results</b></td>
				<td align="right">&nbsp;
					<?php if($vars['previousNextLinks'] != ""):?>
						<?=$vars['previousNextLinks'];?>
					<?php endif;?>		
				</td>
			</tr>
			<tr><td colspan="2" style="line-height:2px;">&nbsp;</td></tr>
		</table>

		<table border="0" cellpadding="3" cellspacing="1" align="center" width="95%" ID="Table1">
			<tr>
				<th>&nbsp;</th>
				<th align=left><a href="products.php?list_products=true&sortby=category_link"><font color=white>Category</font></a></th>
				<th><a href="products.php?list_products=true&sortby=sku&catid=<?=$vars['catid'];?>"><font color=white>SKU</font></a></th>
				<th><a href="products.php?list_products=true&sortby=name&catid=<?=$vars['catid'];?>"><font color=white>Name</font></a></th>
				<th><a href="products.php?list_products=true&sortby=price&catid=<?=$vars['catid'];?>"><font color=white>Price</font></a></th>
				<th><a href="products.php?list_products=true&sortby=display_product&catid=<?=$vars['catid'];?>"><font color=white>Display</font></a></th>
				<th colspan=2>Manage</th>
			</tr>

			<?php $counter = 0;?>
			<?php foreach($data as $index=>$row):?>
				<?php
					$pid = $row['pid'];
					$itemNum = $row['sku'];
					$name = stripslashes($row['name']);
					$displayIt = $row['display_product'];
					$category = $row['catlinks'];
					$price = $_Common->calculateQuantityPrice($row['price']);
					$counter++;
				?>
				<tr bgcolor="<?=$color[$ck = ~$ck];?>">
					<td valign=top align=right width="20"><?=trim($counter);?></td>
					<td valign=top nowrap>&nbsp;<?=$category;?></td>
					<td valign=top><a href="products.php?edit=true&pid=<?=$pid;?>"><?=$itemNum;?></a></td>
					<td valign="top" width="40%" nowrap><?=$name;?></td>
					<td valign=top align="right" nowrap><?=$_Common->format_number($price);?></td>
					<td align=center valign="top"><?=$displayIt;?></td>
					<td align=center valign="middle" width="50"><a href="products.php?edit=true&pid=<?=$pid;?>"><img src="icons/txt.gif" border="0" alt="Edit"></a></td>
					<td align=center valign="middle" width="50"><a href="products.php?delete=true&pid=<?=$pid;?>&catid=<?=$vars['catid'];?>&mid=<?=$row['mid'];?>" onClick="return confirm('Are you sure you want to delete this product?')"><img src="icons/trash.gif" border="0" alt="Delete"></a></td>
				</tr>
			<?php endforeach;?>
			<tr><td colspan=7>&nbsp;</td></tr>
			<tr><td colspan=7><b>Total items returned = </b><?=$vars['rsCount'];?></td></tr>
		</table>
	</form>

<?php endif;?>

</div>
<p>&nbsp;</p>
</body>
</html>






