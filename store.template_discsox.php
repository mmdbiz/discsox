<?php 
  //initialize variables  
  $bodyClass = null;
  $cartVersion = "qs30";
  $cartBreadCrumbName = "Shopping";
  $initializeCart = true;  
  $showMiniCart	= true;
  $inclAnimation = false;
  $dbAccessRequired = true;
  $discontinued = false;
  $checkout_tool_tip = '';
  $currentcategory = '';
  $currentcategory2 = '';
  $useToolTip = false;
  $useZoom = false;
  $useGallery = false;
  $useInfo = false;
  $infoTitle = "How does it work?";
  $infoTarget = "";
  $usePinterest = true;
  $pinterestDescription = 'DiscSox Media Storage Solutions';
  $mainItemPicture = 'snap_fit_kit_and_ext_hidef_pro';
  $mainItemPictureAbsolute  = 'https://mmdesign.com/_images/glamor/snap_fit_kit_and_ext_hidef_pro_750.jpg';
  $new_product = false;
  $infoTitle = "How does it work?";
  $infoTarget = "build-a-kit.php";
  $inclBreadCrumbs = true;
  $dirPrefix = "";
  $currentSubDir = "";
  $currentFileName = "";
  $replaceDescriptionStrings = false;
  $oldDescriptionStrings = "";
  $newDescriptionStrings = "";
  $socialIconsTight = true;
  $snapTrayOnly = false;
  $showAddToCartInSummary = false;
  $summary_add_to_cart = true;
  $summary_detail_link = true;
  $wholesale = false;
  $tight_summary = false;
  $tight_related = false;
  $showKit = false;
  $showAddExtraSleevesTitle = false;
  $forceHiDefDividers = false;
  $hideMetalTray = false;

  //Meta
  $metaDescription = "DiscSox Media Storage Solutions: Store 80 Blu-ray, DVD, CD, Data, Photo CD discs in 12in with Archival Storage sleeves, instant FlipFile access, maximum protection, organizes alphabetically/categorically, minimizes Storage Space";
  $metaKeywords = "CD, CD Storage, DVD, DVD Storage, Blu-ray Disc, Blu-ray Storage, DVD Sleeves, CD Sleeves, Data, Data Storage, HD-DVD, Compact Disc, CD ROM, Compact Disc, Compact Disc Storage, CD ROM Storage, ";

  //microdata

  $itemtype = "LocalBusiness";
  $itemtype = "Product";
  //use page name for structured data as a default
  $struct_detail_page_title = str_replace("-", " ", basename ($_SERVER['REQUEST_URI'], ".php"));
  //use this description for structured data as a default
  $struct_description = "DiscSox Media Storage Solutions: Store 80 Blu-ray, DVD, CD, Data, Photo CD discs in 12in with Archival Storage sleeves, instant FlipFile access, maximum protection, organizes alphabetically/categorically, minimizes Storage Space";
  $struct_retail_price = 0;
  
  $debug = false; 
  
  function getDirPrefix () {
      global $debug;
      global $path;
	  global $dirPrefix ;
	  //check working directory and set path accordingly  
	  //check whether we are on the localhost (discsox) or in the "new" temp dir on the server
	  //adjust the directory level accordingly
	  if(preg_match("/(discsox)/", $path)){
		  $dirLevelAdj = 3;
	  } else {
		  $dirLevelAdj = 2;
	  }
	  $parts = explode('/',$path);   
	  //count parts of PHP_SELF and adjust for dirLevel then count and set "../" accordingly
	  for($i=(count(explode("/",$path))-$dirLevelAdj);$i>0;$i--) $dirPrefix .= "../"; 
	  
	  if($debug) {
		echo ("dirPrefix: " . $dirPrefix ."<br>" );
		echo ("# of parts: " . count(explode("/",$_SERVER["PHP_SELF"])) ."<br>" );
		echo ("dirlevel: " . (count(explode("/",$path))-$dirLevelAdj) ."<br>" );
		echo ("path: " . $path ."<br>" );
	  }
  }  
  $path = $_SERVER["PHP_SELF"];	
  
  if(preg_match("/\b(wholesale)\b/", $path)){
	  $cartVersion = "qs30_2";
	  $cartBreadCrumbName = "Wholesale";
	  //$initializeCart = false;
	  //$showMiniCart	= false;
  }
  if(preg_match("/\b(qs30|qs30_2)\b/", $path)){
	  $dirPrefix = '../';
  } else {
	  getDirPrefix();
  } 
  include $dirPrefix . '_includes/vars.php';  
  include $dirPrefix . '_includes/sitefunctions.php';

  // save wholesale user session var  
  if(preg_match("/\b(wholesale\/index.php)\b/", $path)){
      include_once("../_includes/user_access.inc");
	  $ws_user = $_SESSION['ws_user'];
  }
  
  //if(($initializeCart) && !(preg_match("/\b(qs30)\b/", $path))){
  if(($initializeCart) && !(preg_match("/\b(qs30|qs30_2)\b/", $path))){
	// initialize the program and read the config
	// include_once($dirPrefix . "qs30/include/initialize.inc");
	include_once($dirPrefix . $cartVersion . "/include/initialize.inc");
	$init = new Initialize();
	
	// get the login class and see if required
	$login = $_Registry->LoadClass('login');
	$login->checkLogin();
	
	// load the cart so we can display mini-cart
	$cart = $_Registry->LoadClass('cart');
	$cart->initialize();
	// Minicart
	$miniCart['item_count'] = empty($_Totals['totalQuantity']) ? "0" : $_Totals['totalQuantity'];
	$miniCart['total'] = empty($_Totals['subtotal']) ? "0.00" : number_format($_Totals['subtotal'],2);
  }
  
  // retrieve wholesale user session var
  if(preg_match("/\b(wholesale\/index.php)\b/", $path)){
	$_SESSION['ws_user'] = $ws_user;
  }   
?>
<!DOCTYPE html>
<html lang="en"><!-- InstanceBegin template="/Templates/mainTemplate.dwt.php" codeOutsideHTMLIsLocked="true" -->
<head>
  <!-- InstanceBeginEditable name="VariableDefinitions" -->
<?php 
  // Include and instantiate the mobile detect class.
  require_once '../_includes/Mobile_Detect.php';
  $detect = new Mobile_Detect;
   
  // Any mobile device (phones or tablets).
  // this is used in the store email to track where orders are coming from
  if ( $detect->isMobile() ) {
	 $_SESSION['Display_Device']='mobile';
  } else {
	 $_SESSION['Display_Device']='desktop';
  }

  $dbAccessRequired = false;
  $inclAnimation = false;
  $useToolTip = true;
  $useInfo = false;
  $infoTitle = "How does it work?";
  $infoTarget = "build-a-kit.php";
  $inclBreadCrumbs = true;
  $usePinterest = false;
  $initializeCart = false;
  
  $debug = false;
?>
  <!-- InstanceEndEditable -->

<?php 
  
	  //get current page URL for Pinterest and Google+
	 $pageURL = 'http';
	 if (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == 'on') {$pageURL .= "s";}
	 $pageURL .= "://";
	 if ($_SERVER["SERVER_PORT"] != "80") {
	  $pageURL .= $_SERVER["SERVER_NAME"].":".$_SERVER["SERVER_PORT"].$_SERVER["REQUEST_URI"];
	 } else {
	  $pageURL .= $_SERVER["SERVER_NAME"].$_SERVER["REQUEST_URI"];
	 }
	 if ($debug) {
		 echo ("pageURL: " . $pageURL ."<br>" );
	 }
  if(isset($usePinterest) && ($usePinterest == "true")) {
	//required for Pinterest Link  
    $mainItemPictureAbsolute = $imageFolderPath.'glamor/'.$mainItemPicture.'_750.jpg'; 
    if ($debug) {
       echo ("mainItemPictureAbsolute: " . $mainItemPictureAbsolute ."<br>" );
    }
  }
  if($dbAccessRequired) {
	include_once($dirPrefix . "_includes/db_access.inc");
  }
  if($debug) {
    echo ("imageFolderPath: " . $imageFolderPath  ."<br>");
    echo ("inclAnimation: " . $inclAnimation  ."<br>");
    echo ("dbAccessRequired: " . $dbAccessRequired  ."<br>");
	echo ("This is used for structured data name default <br>");
	echo ("FILENAME sanitized: " . str_replace("-", " ", basename ($_SERVER['REQUEST_URI'], ".php")) ."<br>");
  }
  
?>
<!-- InstanceBeginEditable name="doctitle" -->
<title>DiscSox Media Storage Solutions</title>
<!-- InstanceEndEditable -->

<meta charset="utf-8">
<meta http-equiv="X-UA-Compatible" content="IE=edge">
<meta name="viewport" content="width=device-width, initial-scale=1">
<?php
   // Get the product data by the current SKU
	if (isset($currentsku) && $currentsku != "") {
		$row = get_prod_row_by_sku($currentsku);
		if ($row) {
			extract($row);
			//Get the custom product data
			extract(get_prod_custom_data_by_pid($pid));
			//use meta description if set otherwise use reg. description
			if ($detail_meta_description != "") {
				$metaDescription = $detail_meta_description;
			} else {
				$metaDescription =  strip_tags($description);				
			}
			//Use meta keywords if set otherwise use default
			if ($detail_meta_keywords != "") {
				$metaKeywords = $detail_meta_keywords;
			}
		}
		else {	
			echo('<h1>Error! - SKU does not exist!!! </h1>');
		}
	}
?>
<meta name="description" content="<?=$metaDescription;?>">
<meta name="keywords" content="<?=$metaKeywords;?>">

<!-- Bootstrap -->
<?php if(!$production):?> 
    <link href="../_css/bootstrap.css" rel="stylesheet">
    <link href="../_css/responsive.css" rel="stylesheet" type="text/css">
    <link href="../_css/nav.css" rel="stylesheet" type="text/css">    
    <link href="../_css/general.css" rel="stylesheet" type="text/css">
    <link href="../_css/product.css" rel="stylesheet" type="text/css">
    <!-- SmartMenus jQuery Bootstrap Addon CSS -->
    <link href="../_css/jquery.smartmenus.bootstrap.css" rel="stylesheet">
    <link href="https://fonts.googleapis.com/css?family=Lato:300,400" rel="stylesheet"> 
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/font-awesome/4.5.0/css/font-awesome.min.css">
    <!-- Zoom lens jquery addon -->
    <link rel="stylesheet" type="text/css" href="../_css/jquery.simpleLens.css">
    <link rel="stylesheet" type="text/css" href="../_css/jquery.simpleGallery.css"> 
<?php else:?>
    <link href="../_css/min.css" rel="stylesheet" type="text/css">
<?php endif;?>

<link rel="apple-touch-icon" sizes="180x180" href="/apple-touch-icon.png">
<link rel="icon" type="image/png" href="/favicon-32x32.png" sizes="32x32">
<link rel="icon" type="image/png" href="/favicon-16x16.png" sizes="16x16">
<link rel="manifest" href="/manifest.json">
<link rel="mask-icon" href="/safari-pinned-tab.svg" color="#5bbad5">
<meta name="theme-color" content="#ffffff">

<!-- Bootstrap URL-->
<!-- HTML5 shim and Respond.js for IE8 support of HTML5 elements and media queries -->
<!-- WARNING: Respond.js doesn't work if you view the page via file:// -->
<!--[if lt IE 9]>
      <script src="https://oss.maxcdn.com/html5shiv/3.7.2/html5shiv.min.js"></script>
      <script src="https://oss.maxcdn.com/respond/1.4.2/respond.min.js"></script>
    <![endif]--> 
    
<!--Initialize isMobile var-->
<script>var isMobile = false;</script>
<?php if($useZoom):?> 
	<script>
    //mobile detection for hiding zoom
        !function(a){var b=/iPhone/i,c=/iPod/i,d=/iPad/i,e=/(?=.*\bAndroid\b)(?=.*\bMobile\b)/i,f=/Android/i,g=/(?=.*\bAndroid\b)(?=.*\bSD4930UR\b)/i,h=/(?=.*\bAndroid\b)(?=.*\b(?:KFOT|KFTT|KFJWI|KFJWA|KFSOWI|KFTHWI|KFTHWA|KFAPWI|KFAPWA|KFARWI|KFASWI|KFSAWI|KFSAWA)\b)/i,i=/IEMobile/i,j=/(?=.*\bWindows\b)(?=.*\bARM\b)/i,k=/BlackBerry/i,l=/BB10/i,m=/Opera Mini/i,n=/(CriOS|Chrome)(?=.*\bMobile\b)/i,o=/(?=.*\bFirefox\b)(?=.*\bMobile\b)/i,p=new RegExp("(?:Nexus 7|BNTV250|Kindle Fire|Silk|GT-P1000)","i"),q=function(a,b){return a.test(b)},r=function(a){var r=a||navigator.userAgent,s=r.split("[FBAN");return"undefined"!=typeof s[1]&&(r=s[0]),s=r.split("Twitter"),"undefined"!=typeof s[1]&&(r=s[0]),this.apple={phone:q(b,r),ipod:q(c,r),tablet:!q(b,r)&&q(d,r),device:q(b,r)||q(c,r)||q(d,r)},this.amazon={phone:q(g,r),tablet:!q(g,r)&&q(h,r),device:q(g,r)||q(h,r)},this.android={phone:q(g,r)||q(e,r),tablet:!q(g,r)&&!q(e,r)&&(q(h,r)||q(f,r)),device:q(g,r)||q(h,r)||q(e,r)||q(f,r)},this.windows={phone:q(i,r),tablet:q(j,r),device:q(i,r)||q(j,r)},this.other={blackberry:q(k,r),blackberry10:q(l,r),opera:q(m,r),firefox:q(o,r),chrome:q(n,r),device:q(k,r)||q(l,r)||q(m,r)||q(o,r)||q(n,r)},this.seven_inch=q(p,r),this.any=this.apple.device||this.android.device||this.windows.device||this.other.device||this.seven_inch,this.phone=this.apple.phone||this.android.phone||this.windows.phone,this.tablet=this.apple.tablet||this.android.tablet||this.windows.tablet,"undefined"==typeof window?this:void 0},s=function(){var a=new r;return a.Class=r,a};"undefined"!=typeof module&&module.exports&&"undefined"==typeof window?module.exports=r:"undefined"!=typeof module&&module.exports&&"undefined"!=typeof window?module.exports=s():"function"==typeof define&&define.amd?define("isMobile",[],a.isMobile=s()):a.isMobile=s()}(this);
    </script>
<?php endif;?>

<!-- InstanceBeginEditable name="head" -->  
    <link href="../_css/cart.css" rel="stylesheet" type="text/css"> 
<!-- InstanceEndEditable -->
</head>
<body<?php if($inclAnimation):?> onload="init();"<?php endif;?> <?php if (isset($bodyClass)) { echo "class='" . $bodyClass ."'"; } ?>>
	<div class="container-fluid">
	  <div class="row">
		<div class="col-sm-3 col-xxs-6 col-xs-4">
		  <h1 class="header-title"><a href="../index.php" class="text-danger">DiscSox</a></h1>
		</div>
		<div class="text-center  col-md-7 hidden-xxs col-sm-7 col-lg-7 col-xs-5 header-tagline">
		  <div class="hidden-xs header-tagline-spacing">&nbsp;</div>
		<a href="../media-storage-high-capacity.php">Media Storage</a> Solutions for <a href="../Movies/dvd-blu-ray-storage.php">DVD</a>, <a href="../Movies/Sleeves/hidef-pro-poly-sleeve.php">Blu-ray</a>, <a href="../Music/cd-storage.php">CD</a>, <a href="../Games/game-storage.php">Game</a>, <a href="../Data/data-storage.php">Data</a> Discs &amp; <a href="../Vinyl-LP-Storage/vinyl-lp-storage.php">Vinyl LPs</a></div>
		<div class="col-sm-2 col-xxs-6 col-xs-3"><a href="../index.php"><img class="img-responsive pull-right" width="71" height="50" src="../_images/logos/ds_logo_68.png" alt="Responsive image"></a></div>
		<div class="col-xs-12 text-center header-divider"><span class="header-divider-text">Less is More</span></div>
	  </div> 
	</div>
	<div class="nav-container">
	  <nav class="navbar navbar-default navbar-default  col-lg-12">
		<div class="container-fluid">
		  <div class="row">
			<!-- Brand and toggle get grouped for better mobile display -->
			<div class="navbar-header">
			  <button type="button" class="navbar-toggle collapsed" data-toggle="collapse" data-target="#topFixedNavbar1" aria-expanded="false">
				<span class="sr-only">Toggle navigation</span>
				<span class="icon-bar top-bar"></span>
				<span class="icon-bar middle-bar"></span>
				<span class="icon-bar bottom-bar"></span>
			  </button>
			  <!--<a class="navbar-brand" href="#"></a>-->
			  <div class="navbar-search"  lang="en">
				<a href="#modalSearch" data-toggle="modal" data-target="#modalSearch"  class="navbar-search-link ">
				  <span id="searchGlyph" class="glyphicon glyphicon-search"></span>
				</a>
				<div class="navbar-search-text"></div>
			  </div>
			  <div class="navbar-deals"  lang="en"> 
				<a href="../deals.php" class="navbar-deals-link ">
				  <span id="deals" class="glyphicon glyphicon-tags"></span> &nbsp;Deals
				</a>
			  </div>
			  <div class="hidden-sm hidden-md  hidden-lg pull-right">
				<a href="<?=$dirPrefix;?><?=$cartVersion;?>/myaccount.php" class="navbar-right-icons"><span class="glyphicon glyphicon-user"></span></a>
				<a href="<?=$dirPrefix;?><?=$cartVersion;?>/cart.php" class="navbar-right-icons"><span class="glyphicon glyphicon-shopping-cart cartIcon"></span><span class="cartItemCount"><?=$miniCart['item_count'];?></span></a>
			  </div>
			</div>

			<!-- Collect the nav links, forms, and other content for toggling -->
			<div class="collapse navbar-collapse " id="topFixedNavbar1">
			  <ul class="nav navbar-nav">
				<li class="dropdown center"><a href="../index.php" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false"><span class="navLeftMainTitle">SHOP</span><br>
				  STORAGE SOLUTIONS<span class="caret"></span></a>
				  <ul class="dropdown-menu">
					<li><a href="../Movies/dvd-blu-ray-storage.php">MOVIES<span class="caret"></span></a>            
					  <ul class="dropdown-menu mega-menu">
						<li>
						  <div class="megaMenu">
							<div class="megaMenuCol ">
							  <div class="megaMenuPanel"> 
								<a href="../Movies/Kits/dvd-storage-kits.php"><span class="megaMenuTitle">DVD Storage Kits</span></a>
								<a href="../Movies/Kits/dvd-pro-poly-set.php">
								<span class="megaMenuItem">DVD Pro Kit</span>
									<span class="megaMenuSubText">Build a customized kit to fit your space</span>
								</a>
								<a href="../Movies/Kits/hidef-pro-poly-set.php">
								<span class="megaMenuItem">HiDef Pro Kit</span>
									<span class="megaMenuSubText">Store 65+ Blu-ray movies in 12 inches</span>
								</a>
								<a href="../Movies/Kits/dvd2-set.php">
								<span class="megaMenuItem">DVD2 Kit - Discontinued!</span>
									<span class="megaMenuSubText">Store 65+ DVD movies in 12 inches</span>
								</a>
							  </div>
							<div class="megaMenudivider">&nbsp;</div>
							  <div class="megaMenuPanel"> 
								<a href="../Movies/Sleeves/dvd-storage-sleeves.php"><span class="megaMenuTitle">DVD Storage Sleeves</span></a>
								<a href="../Movies/Sleeves/dvd-pro-poly-sleeve.php">
									<span class="megaMenuItem">DVD Pro Sleeves</span>
									<span class="megaMenuSubText">Archival quality sleeves that hold 2 discs</span>
								</a>
								<a href="../Movies/Sleeves/hidef-pro-poly-sleeve.php">
									<span class="megaMenuItem">HiDef Pro Sleeves</span>
									<span class="megaMenuSubText">Sleeves hold all contents of original packaging</span>
								</a>
								<a href="../Movies/Sleeves/4-disc-wallet.php">
									<span class="megaMenuItem">4-Disc Wallet</span>
									<span class="megaMenuSubText">Wallet holds 4 discs and fits into DiscSox Sleeves</span>
								</a>
								<a href="../Movies/Sleeves/dvd2-sleeve.php">
									<span class="megaMenuItem">DVD2 Sleeves - Discontinued!</span>
									<span class="megaMenuSubText">Sleeves hold all contents - compact design</span>
								</a>
							  </div> 
							</div>
							<div class="megaMenuCol">
							  <div class="megaMenuPanel">
								<a href="../Accessories/dvd-accessories.php"><span class="megaMenuTitle">DVD Accessories</span></a>
								<a href="../media-storage-cabinets.php">
                                    <span class="megaMenuItem">Media Cabinets</span>
									<span class="megaMenuSubText">Store 1750+ movies in a heavy duty steel cabinet</span>
								</a>
								<a href="../media-storage-chests.php">
                                    <span class="megaMenuItem">Media Chests</span>
									<span class="megaMenuSubText">Store 820+ movies in cutomizable wood chest</span>
								</a>
								<a href="../Cases/cd-dvd-dj-cases.php#collapseFour">
                                    <span class="megaMenuItem">Media/DJ Cases</span>
									<span class="megaMenuSubText">Take your collection on the road</span>
								</a>
								<a href="../Decorative-Cases/decorative-media-cases.php">
								<span class="megaMenuItem">Decorative Cases</span>
									<span class="megaMenuSubText">Savor the past and store 300+ movies</span>
								</a>
								<a href="../Movies/Holders/dvd-holders.php">
                                    <span class="megaMenuItem">DVD Holders</span>
									<span class="megaMenuSubText">Expand Snap-fit DVD Holders to any Length</span>
								</a>
								<a href="../Accessories/Media-Catalog-Software/dvd-movie-catalog-database-software.php">
                                    <span class="megaMenuItem">DVD Catalog Software</span>
									<span class="megaMenuSubText">Organize Movie Collection with Database SW</span>
								</a>
							  </div>
							</div>
							<div class="megaMenuMoviePic">
							  <a href="../Movies/dvd-blu-ray-storage.php"><img src="../_images/movie_storage_250.png" width="399" height="250" alt=""/></a>
							</div>
						  </div>
						</li>
					  </ul>
					</li>
					<li><a href="../Music/cd-storage.php">MUSIC<span class="caret"></span></a>            
					  <ul class="dropdown-menu mega-menu">
						<li>
						  <div class="megaMenu">
							<div class="megaMenuCol ">
							  <div class="megaMenuPanel"> 
								<a href="../Music/Kits/cd-storage-kits.php"><span class="megaMenuTitle">CD Storage Kits</span></a>
								<a href="../Music/Kits/cd-pro-poly-set.php">
								<span class="megaMenuItem">CD Pro Kit</span>
									<span class="megaMenuSubText">Build a customized kit to fit your space</span>
								</a>
								<a href="../Music/Kits/cd-standard-plus-set.php">
								<span class="megaMenuItem">CD Standard Plus Kit</span>
									<span class="megaMenuSubText">Store 75+ CDs in 12 inches</span>
								</a>
								<a href="../Music/Kits/cd-standard-set.php">
								<span class="megaMenuItem">CD Standard Kit</span>
									<span class="megaMenuSubText">Store 75+ CDs in 12 inches</span>
								</a>
								<a href="../Music/Kits/classic-poly-set.php">
								<span class="megaMenuItem">Classic Kit</span>
									<span class="megaMenuSubText">Store 75+ CDs in 12 inches</span>
								</a>
							  </div>
							<div class="megaMenudivider">&nbsp;</div>
							  <div class="megaMenuPanel"> 
								<a href="../Music/Sleeves/cd-storage-sleeves.php"><span class="megaMenuTitle">CD Storage Sleeves</span></a>
								<a href="../Music/Sleeves/cd-pro-poly-sleeve.php">
									<span class="megaMenuItem">CD Pro Sleeves</span>
									<span class="megaMenuSubText">Archival quality sleeves that hold all content</span>
								</a>
								<a href="../Music/Sleeves/cd-standard-plus-sleeve.php">
									<span class="megaMenuItem">CD Standard Plus Sleeves</span>
									<span class="megaMenuSubText">Sleeves hold 2 discs and booklet</span>
								</a>
								<a href="../Music/Sleeves/cd-standard-sleeve.php">
									<span class="megaMenuItem">CD Standard Sleeves</span>
									<span class="megaMenuSubText">Sleeves hold 2 discs and booklet</span>
								</a>
								<a href="../Music/Sleeves/4-disc-cd-wallet.php">
									<span class="megaMenuItem">4-Disc CD Wallet</span>
									<span class="megaMenuSubText">Wallet holds 4 discs, booklet and tray card</span>
								</a>
								<a href="../Music/Sleeves/classic-poly-sleeve.php">
									<span class="megaMenuItem">Classic Sleeves</span>
									<span class="megaMenuSubText">Sleeves hold 2 discs and booklet</span>
								</a>
								<a href="../Music/Sleeves/j-box-sleeve.php">
									<span class="megaMenuItem">J-Box Sleeves</span>
									<span class="megaMenuSubText">Holds booklet &amp; tray card (for CD Juke Boxes)</span>
								</a>
							  </div>
							</div>
							<div class="megaMenuCol">
							  <div class="megaMenuPanel">
								<a href="../Accessories/cd-accessories.php"><span class="megaMenuTitle">CD Accessories</span></a>
								<a href="../media-storage-cabinets.php">
                                    <span class="megaMenuItem">Media Cabinets</span>
									<span class="megaMenuSubText">Store 2500+ CDs in a heavy duty steel cabinet</span>
								</a>
								<a href="../media-storage-chests.php">
                                    <span class="megaMenuItem">Media Chests</span>
									<span class="megaMenuSubText">Store 950+ CDs in cutomizable wood chest</span>
								</a>
								<a href="../Cases/cd-dvd-dj-cases.php#collapseFive">
                                    <span class="megaMenuItem">Media/DJ Cases</span>
									<span class="megaMenuSubText">Take your collection on the road</span>
								</a>
								<a href="../Decorative-Cases/decorative-media-cases.php">
                                    <span class="megaMenuItem">Decorative Cases</span>
									<span class="megaMenuSubText">Savor the past and store 360+ CDs</span>
								</a>
								<a href="../Music/Holders/cd-holders.php">
                                    <span class="megaMenuItem">CD Holders</span>
									<span class="megaMenuSubText">Expand Snap-fit HOlders to any Length</span>
								</a>
								<a href="../Accessories/Media-Catalog-Software/cd-music-catalog-database-software.php">
                                    <span class="megaMenuItem">CD Catalog Software</span>
									<span class="megaMenuSubText">Organize Music Collection with Database SW</span>
								</a>
							  </div>
								
							<div class="megaMenudivider">&nbsp;</div>
							  <div class="megaMenuPanel"> 
								<a href="../Vinyl-LP-Storage/vinyl-lp-storage.php"><span class="megaMenuTitle">Vinyl LP Storage</span></a>
								<a href="../vinyl-storage-cabinets.php">
									<span class="megaMenuItem">Vinyl LP Cabinets</span>
								</a>
								<a href="../vinyl-storage-chests.php">
									<span class="megaMenuItem">Vinyl LP Chests</span>
								</a>
								<a href="../vinyl-storage-cases.php">
									<span class="megaMenuItem">Vinyl LP Cases</span>
								</a>
								</div>
							</div>    
							<div class="megaMenuMusicPic">
							  <a href="../Music/cd-storage.php"><img src="../_images/music_storage_250.png" width="399" height="250" alt=""/></a>
							</div>
						  </div>
						</li>
					  </ul>
					</li>
					<li><a href="../Games/game-storage.php">GAMES<span class="caret"></span></a>
					  <ul class="dropdown-menu mega-menu">
						<li>
						  <div class="megaMenu">
							<div class="megaMenuCol ">
							  <div class="megaMenuPanel"> 
								<a href="../Games/Kits/game-storage-kits.php"><span class="megaMenuTitle">Game Storage Kits</span></a>
								<a href="../Games/Kits/game-pro-set.php">
								<span class="megaMenuItem">Wii, XBOX(360), PS2, PS4, PS5</span>
									<span class="megaMenuSubText">Build a customized kit to fit your space</span>
								</a>
								<a href="../Games/Kits/game-pro-ps3-set.php">
								<span class="megaMenuItem">PS3 Kit</span>
									<span class="megaMenuSubText">Store 65+ PS3 Games in 12 inches</span>
								</a>
							  </div>
							<div class="megaMenudivider">&nbsp;</div>
							  <div class="megaMenuPanel"> 
								<a href="../Games/Sleeves/game-storage-sleeves.php"><span class="megaMenuTitle">Game Storage Sleeves</span></a>
								<a href="../Games/Sleeves/game-storage-sleeves.php">
									<span class="megaMenuItem">Wii, XBOX(360), PS2, PS4, PS5</span>
									<span class="megaMenuSubText">Archival quality sleeves that hold 2 games</span>
								</a>
								<a href="../Games/Sleeves/game-pro-ps3-sleeve.php">
									<span class="megaMenuItem">PS3</span>
									<span class="megaMenuSubText">Sleeves hold all contents of original game</span>
								</a>
							  </div> 
							</div>
							<div class="megaMenuCol">
							  <div class="megaMenuPanel">
								<a href="../Accessories/game-accessories.php"><span class="megaMenuTitle">Game Accessories</span></a>
								<a href="../media-storage-cabinets.php">
								<span class="megaMenuItem">Media Cabinets</span>
									<span class="megaMenuSubText">Store 1750+ games in a heavy duty steel cabinet</span>
								</a>
								<a href="../media-storage-chests.php">
								<span class="megaMenuItem">Media Chests</span>
									<span class="megaMenuSubText">Store 820+ games in cutomizable wood chest</span>
								</a>
								<a href="../Cases/cd-dvd-dj-cases.php#collapseFour">
								<span class="megaMenuItem">Media/DJ Cases</span>
									<span class="megaMenuSubText">Take your games on the road</span>
								</a>
								<a href="../Decorative-Cases/decorative-media-cases.php">
								<span class="megaMenuItem">Decorative Cases</span>
									<span class="megaMenuSubText">Savor the past and store 360+ Games</span>
								</a>
								<a href="../Games/Storage-Trays/game-storage-trays.php">
								<span class="megaMenuItem">Storage Trays</span>
									<span class="megaMenuSubText">Expand Snap-fit Storage Trays to any Length</span>
								</a>
							  </div>
							</div>
							<div class="megaMenuGamePic">
							  <a href="../Games/game-storage.php"><img src="../_images/game_storage_250.png" width="399" height="250" alt=""/></a>
							</div>
						  </div>
						</li>
					  </ul>
					</li>
					<li><a href="../Data/data-storage.php">DATA<span class="caret"></span></a>            
					  <ul class="dropdown-menu mega-menu">                    
						<li>
						  <div class="megaMenu">
							<div class="megaMenuCol ">
							  <div class="megaMenuPanel"> 
								<a href="../Data/Kits/data-storage-kits.php"><span class="megaMenuTitle">Data Storage Kits</span></a>
								<a href="../Data/Kits/dataplus-set.php">
								<span class="megaMenuItem">Data Plus Kit</span>
									<span class="megaMenuSubText">Build a customized kit to fit your space</span>
								</a>
								<a href="../Data/Kits/datadouble-set.php">
								<span class="megaMenuItem">Data Double Kit</span>
									<span class="megaMenuSubText">Build a customized kit to fit your space</span>
								</a>
								<a href="../Data/Kits/data-set.php">
								<span class="megaMenuItem">Data Kit</span>
									<span class="megaMenuSubText">Store 85+ data discs in 12 inches</span>
								</a>
								<a href="../Data/Kits/datafp-set.php">
								<span class="megaMenuItem">DataFP Kit</span>
									<span class="megaMenuSubText">Store 85+ data discs in 12 inches</span>
								</a>
							  </div>
							<div class="megaMenudivider">&nbsp;</div>
							  <div class="megaMenuPanel"> 
								<a href="../Data/Sleeves/data-storage-sleeves.php"><span class="megaMenuTitle">Data Storage Sleeves</span></a>
								<a href="../Data/Sleeves/dataplus-sleeve.php">
									<span class="megaMenuItem">Data Plus Sleeves</span>
									<span class="megaMenuSubText">Archival quality, fully protect all content</span>
								</a>
								<a href="../Data/Sleeves/datadouble-sleeve.php">
									<span class="megaMenuItem">Data Double Sleeves</span>
									<span class="megaMenuSubText">Archival quality, fully protect all content</span>
								</a>
								<a href="../Data/Sleeves/data-sleeve.php">
									<span class="megaMenuItem">Data Sleeves</span>
									<span class="megaMenuSubText">Sleeves hold 1 disc and booklet</span>
								</a>
								<a href="../Data/Sleeves/datafp-sleeve.php">
									<span class="megaMenuItem">DataFP Sleeves</span>
									<span class="megaMenuSubText">Includes flap to close sleeve</span>
								</a>
							  </div>
							</div>
							<div class="megaMenuCol">
							  <div class="megaMenuPanel">
								<a href="../Accessories/data-accessories.php"><span class="megaMenuTitle">Data Accessories</span></a>
								<a href="../media-storage-cabinets.php">
								<span class="megaMenuItem">Media Cabinets</span>
									<span class="megaMenuSubText">Store 3000+ discs in a heavy duty steel cabinet</span>
								</a>
								<a href="../media-storage-chests.php">
								<span class="megaMenuItem">Media Chests</span>
									<span class="megaMenuSubText">Store 1350+ discs in cutomizable wood chest</span>
								</a>
								<a href="../Cases/cd-dvd-dj-cases.php#collapseFive">
								<span class="megaMenuItem">Media/DJ Cases</span>
									<span class="megaMenuSubText">Take your data on the road</span>
								</a>
								<a href="../Decorative-Cases/decorative-media-cases.php">
								<span class="megaMenuItem">Decorative Cases</span>
									<span class="megaMenuSubText">Savor the past and store 375+ discs</span>
								</a>
								<a href="../Data/Storage-Trays/data-storage-trays.php">
								<span class="megaMenuItem">Storage Trays</span>
									<span class="megaMenuSubText">Expand Snap-fit Storage Trays to any Length</span>
								</a>
							  </div>
							</div>    
							<div class="megaMenuDataPic">
							  <a href="../Data/data-storage.php"><img src="../_images/data_storage_250.png" width="399" height="250" alt=""/></a>
							</div>
						  </div>
						</li>
					  </ul>
					</li>
					<li><a href="../media-storage-high-capacity.php">LARGE COLLECTIONS<span class="caret"></span></a>
					  <ul class="dropdown-menu mega-menu">                    
						<li>
						  <div class="megaMenu">
							<div class="megaMenuCol">
							  <div class="megaMenuPanel">
								<a href="../media-storage-high-capacity.php"><span class="megaMenuTitle">Large Collection Solutions</span></a>
								<a href="../media-storage-cabinets.php">
								<span class="megaMenuItem">Media Cabinets</span>
									<span class="megaMenuSubText">Store 2500+ CDs or 1750 movies in steel cabinet</span>
								</a>
								<a href="../media-storage-chests.php">
								<span class="megaMenuItem">Media Chests</span>
									<span class="megaMenuSubText">Store up to 1350 discs in cutomizable wood chest</span>
								</a>
								<a href="../Cases/cd-dvd-dj-cases.php">
								<span class="megaMenuItem">Media/DJ Cases</span>
									<span class="megaMenuSubText">Take your collection on the road</span>
								</a>
								<a href="../Decorative-Cases/decorative-media-cases.php">
								<span class="megaMenuItem">Decorative Cases</span>
									<span class="megaMenuSubText">Savor the past and store 360+ CDs</span>
								</a>
							  </div>
							</div>
							<div class="megaMenuCol">
							  <div class="megaMenuPanel"> 
								<a href="../Vinyl-LP-Storage/vinyl-lp-storage.php"><span class="megaMenuTitle">Vinyl LP Storage</span></a>
								<a href="../vinyl-storage-cabinets.php">
									<span class="megaMenuItem">Vinyl LP Cabinets</span>
								</a>
								<a href="../vinyl-storage-chests.php">
									<span class="megaMenuItem">Vinyl LP Chests</span>
								</a>
								<a href="../vinyl-storage-cases.php">
									<span class="megaMenuItem">Vinyl LP Cases</span>
								</a>
								</div>
							  </div>
							<div class="megaMenuMusicPic">
							  <a href="../media-storage-high-capacity.php"><img src="../_images/combined_large_coll_250.png" width="399" height="250" alt=""/></a>
							</div>
						  </div>
						</li>
					  </ul>
					</li>
					<li><a href="../Accessories/cd-dvd-accessories.php#collapseZero">CUSTOM</a></li>
					<li><a href="../travel-solutions.php">TRAVEL</a></li>
					<li><a href="<?=$dirPrefix;?>Accessories/cd-dvd-accessories.php" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false">ACCESSORIES<span class="caret"></span></a>
					  <ul class="dropdown-menu mega-menu">
						<li>
						  <div class="megaMenu">
							<div class="megaMenuCol">
							  <div class="megaMenuPanel">
								<a href="<?=$dirPrefix;?>Accessories/cd-dvd-accessories.php"><span class="megaMenuTitle">Accessories</span></a>
								<a href="../Accessories/Dividers/dividers.php">
								<span class="megaMenuItem">Dividers</span>
									<span class="megaMenuSubText">Organize Collection Alphabetically or by Category</span>
								</a>
								<a href="../Accessories/cd-dvd-accessories.php#collapseTwo" data-target="#collapseTwo" data-parent="#accordion" data-toggle="collapse" role="button">
								<span class="megaMenuItem openDrawerDivider">Drawer Dividers</span>
									<span class="megaMenuSubText">Separate drawers, trunks, chests into rows</span>
								</a>
								<a href="../Accessories/Holders-Trays/holders-trays.php">
								<span class="megaMenuItem">Holders/Trays</span>
									<span class="megaMenuSubText">Customize your storage solution</span>
								</a>
								<a href="../Accessories/Labels/labels.php" data-target="#collapseThree" data-parent="#accordion" data-toggle="collapse" role="button">
								<span class="megaMenuItem">Labels</span>
									<span class="megaMenuSubText">Customize your Dividers</span>
								</a>
								<a href="../Accessories/cd-dvd-accessories.php#collapseFour" data-target="#collapseFour" data-parent="#accordion" data-toggle="collapse" role="button">
								<span class="megaMenuItem">Maintenance &amp; Cleaning </span>
									<span class="megaMenuSubText">Maintain your discs</span>
								</a>
								<a href="../Accessories/Media-Catalog-Software/media-catalog-software.php" data-target="#collapseFive" data-parent="#accordion" data-toggle="collapse" role="button">
								<span class="megaMenuItem">Media Catalog Software </span>
									<span class="megaMenuSubText">Catalog your collection</span>
								</a>
								<a href="../Accessories/cd-dvd-accessories.php#collapseSix" data-target="#collapseSix" data-parent="#accordion" data-toggle="collapse" role="button">
								<span class="megaMenuItem">Sliders</span>
									<span class="megaMenuSubText">Convert trays to full-extension pull-out units</span>
								</a>
							  </div>
							</div> 
							<div class="megaMenuCol">
							  <div class="megaMenuPanel">
								<a href="../Accessories/data-accessories.php"><span class="megaMenuTitle">&nbsp;</span></a>
								<a href="../Accessories/Spacers/media-spacers.php" data-target="#collapseSix" data-parent="#accordion" data-toggle="collapse" role="button">
								<span class="megaMenuItem">Spacers</span>
									<span class="megaMenuSubText">Make movie cases compatible with music media</span>
								</a>
								<a href="../Accessories/Stoppers/media-stoppers.php" data-target="#collapseSix" data-parent="#accordion" data-toggle="collapse" role="button">
								<span class="megaMenuItem">Stoppers</span>
									<span class="megaMenuSubText">Get media to the front of trays, chests & drawers</span>
								</a>
								<a href="../travel-solutions.php" data-target="#collapseSix" data-parent="#accordion" data-toggle="collapse" role="button">
								<span class="megaMenuItem">Travel</span>
									<span class="megaMenuSubText">Take your DiscSox on the raod</span>
								</a>
							  </div>
								<a href="../Accessories/Wedges/wedges.php" data-target="#collapseSix" data-parent="#accordion" data-toggle="collapse" role="button">
								<span class="megaMenuItem">Wedges</span>
									<span class="megaMenuSubText">Hold media at an angle to allow flip-through</span>
								</a>
							</div>  
							<div class="megaMenuMusicPic">
							  <a href="../Accessories/cd-dvd-accessories.php"><img src="../_images/accessories.png" width="399" height="250" alt=""/></a>
							</div>
						  </div>
						</li>
					  </ul>
					</li>
				  </ul>
				</li>
			  </ul>
			  <ul class="nav navbar-nav navbar-right">
				<li class="dropdown"><a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false"> <span class="glyphicon glyphicon-wrench"></span> Support<span class="caret"></span></a>
				  <ul class="dropdown-menu">
					<li><a href="mailto:orders@discsox.com?subject=Cancel%20Order&body=Please%20complete%20the%20following%20to%20cancel%20your%20order%3A%20%0A%0AOrder%20Number%3A%20%09%0ACustomer%20Name%3A%09%0ACustomer%20Email%3A%09%0AComments%3A%09%0A%0AYou%20will%20receive%20a%20confirmation%20email%20and%20your%20credit%20card%20will%20not%20be%20charged!">Cancel Order</a></li>
					<li><a href="<?=$dirPrefix;?>customer-service.php">Customer Service</a></li>
					<li><a href="#">FAQ<span class="caret"></span></a>
					  <ul class="dropdown-menu">
                        <li><a href="<?=$dirPrefix;?>Support/FAQ/general-faq.php" rel="modal">General FAQ</a>
                        <li><a href="<?=$dirPrefix;?>Support/FAQ/cd-faq.php" rel="modal">CD FAQ</a>
                        <li><a href="<?=$dirPrefix;?>Support/FAQ/dvd-faq.php" rel="modal">DVD FAQ</a>
                        <li><a href="<?=$dirPrefix;?>Support/FAQ/soxchest-faq.php" rel="modal">SoxChest FAQ</a>
                        <li><a href="<?=$dirPrefix;?>Support/FAQ/cabinet-faq.php" rel="modal">Media Cabinet FAQ</a>
                        <li><a href="<?=$dirPrefix;?>Support/FAQ/readerware-faq.php" rel="modal">Readerware FAQ</a>
                      </ul>
                    </li>
					<li><a href="../Support/free-tools.php">Free Tools</a></li>
					<li><a href="../help.php">Help</a></li>
					<li><a href="#">How To Use<span class="caret"></span></a>
					  <ul class="dropdown-menu">
						<li><a href="#">Sleeves<span class="caret"></span></a>
						  <ul class="dropdown-menu">
							<li><a href="<?=$dirPrefix;?>Support/How-To/cd-pro.php" rel="modal">CD Pro</a></li>
							<li><a href="<?=$dirPrefix;?>Support/How-To/classic.php" rel="modal">Classic</a></li>
							<li><a href="<?=$dirPrefix;?>Support/How-To/data.php" rel="modal">Data</a></li>
							<li><a href="<?=$dirPrefix;?>Support/How-To/data-double.php" rel="modal">Data Double</a></li>
							<li><a href="<?=$dirPrefix;?>Support/How-To/datafp.php" rel="modal">DataFP</a></li>
							<li><a href="<?=$dirPrefix;?>Support/How-To/dvd-pro.php" rel="modal">DVD Pro</a></li>
							<li><a href="<?=$dirPrefix;?>Support/How-To/dvd2.php" rel="modal">DVD2</a></li>
							<li><a href="<?=$dirPrefix;?>Support/How-To/game-pro.php" rel="modal">Game Pro</a></li>
							<li><a href="<?=$dirPrefix;?>Support/How-To/game-pro-ps3.php" rel="modal">Game Pro PS3</a></li>
							<li><a href="<?=$dirPrefix;?>Support/How-To/hidef-pro.php" rel="modal">HiDef Pro</a></li>
							<li><a href="<?=$dirPrefix;?>Support/How-To/standard.php" rel="modal">Standard</a></li>
							<li><a href="<?=$dirPrefix;?>Support/How-To/video-cd.php" rel="modal">Video CD</a></li>
						  </ul>                
						</li>
						<li><a href="#">Accessories<span class="caret"></span></a>
						  <ul class="dropdown-menu">
							<li><a href="<?=$dirPrefix;?>Support/How-To/drawer-divider.php" rel="modal">Drawer Divider</a></li>
							<li><a href="<?=$dirPrefix;?>Support/How-To/slider.php" rel="modal">Slider</a></li>
							<li><a href="<?=$dirPrefix;?>Support/How-To/stopper.php" rel="modal">Stopper</a></li>
							<li><a href="<?=$dirPrefix;?>Support/How-To/snap-trays.php" rel="modal">Snap-Fit Tray/Holder</a></li>
						  </ul>                
						</li>
					  </ul>                
					</li>
					<li><a href="../Support/resources.php">Resources</a></li>
					<li><a href="../returns.php">Returns</a></li>
					<li><a href="../track-orders.php">Track Orders</a></li>
					<li><a href="../shipping-and-delivery.php">Shipping & Delivery</a></li>
					<!--<li><a href="http://forum.mmdesign.com/YaBB.pl" target="_blank">User Forum</a></li>-->
				  </ul>
				</li>
				<li class="dropdown">
					<a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false"><span class="glyphicon glyphicon-user"></span>              
					<?php if(isset($_SESSION['isRegistered']) && $_SESSION['isRegistered'] == 'true') :?>
					  <div class="weclomeMsg">Hi <?=$_SESSION['firstname'];?>
					  </div>
					  Your Account
					<?php else:?>
					  Sign In
					<?php endif;?><span class="caret"></span></a>
				  <ul class="dropdown-menu">
					<li><a href="<?=$dirPrefix;?><?=$cartVersion;?>/myaccount.php">Your Account</a></li>
					<!--<li><a href="<?=$dirPrefix;?>Support/my-account.php" rel="modal">My Account modal</a></li>-->
					<li><a href="<?=$dirPrefix;?>wholesale/ws_login.php">Merchant Login</a></li>               
					<?php if(isset($_SESSION['isRegistered']) && $_SESSION['isRegistered'] == 'true') :?>
					  <li><a href="<?=$dirPrefix;?><?=$cartVersion;?>/login.php?logout=1">Not <?=$_SESSION['firstname'];?>? Sign Out</a></li>
					<?php endif;?>
				  </ul>
				</li>
				<?php if(($showMiniCart == 'true')): ?>
				  <li class="cartLink">
					<a href="<?=$dirPrefix;?><?=$cartVersion;?>/cart.php"><span class="glyphicon glyphicon-shopping-cart"></span> <?=$miniCart['item_count'];?> In Cart</a> 
					<div class="miniCart "> 
					  <div class="minicartItems"><?=$miniCart['item_count'];?> item<?php if($miniCart['item_count'] >1):?>s<?php endif;?> in your cart </div>
					  <div class="minicartSubtotal">Subtotal: <?=$miniCart['total'];?>  
					  </div>
					  <div class="tiny">Tax & shipping costs, discounts, and
						GiftCards will be applied during checkout</div>
						<div class="submitButtonContainer m-t-05">
						<div class="minicartLink">
						  <a href="<?=$dirPrefix;?><?=$cartVersion;?>/cart.php">
							<!--<img src="../_images/buttons/arrow_button_right.png" width="40" height="40" alt=""/></a><a class="largeSubmitButton p-b-03" href="../qs30/cart.php">review cart & checkout</a></div>-->
							<button class="btn btn-danger m-l-05" type="button">
								review cart & checkout
								<span class="glyphicon glyphicon-triangle-right" aria-hidden="true"></span>
							</button>
						  </a>
						</div>
					  </div>
					</div>
				  </li>
				<?php endif;?>
			  </ul>
			</div>
			<!-- /.navbar-collapse --></div>
		  </div>
		<!-- /.container-fluid -->
		</nav>

	   <!-- Breadcrumbs -->
	  <?php if($inclBreadCrumbs):?>  
		<div class="row m-x-0">
		  <div class="col-xs-12">
			<div class="breadCrumbs">
			  <?php 
					//link sub directory names to specific pages
	//		        $dirNames = array("Movies", "Music", "Games", "Data", "Photo", "Large Collections");
	//				$linkNames = array("Movies/dvd-blu-ray-storage.php", "Music/cd-storage.php", "Games/game-storage.php", "Data/data-storage.php", "Photo", "/Large-Collections/media-storage-high-capacity.php");
					$path = $_SERVER["PHP_SELF"];				
					if($debug) {
					  echo ("path: " . $path ."<br>" );
					}
					$parts = explode('/',$path);
					if (count($parts) < 2) {
						// we are at the top level
						echo('<i class="fa fa-home" aria-hidden="true"></i>');
					} else {
						echo ("<a href=\"/\"><i class='fa fa-home' aria-hidden='true'></i></a> <span class='breadCrumbsSeparator'>&rang;</span> ");
						for ($i = 1; $i < count($parts); $i++) {
							// Check that there is no dot in the name, 
							// then it's not the file name but a directory
							if (!strstr($parts[$i],".")) {
								echo("<a href=\"");
								// turn the directory name into directory link
								// assumes that this file exists!!!!
								for ($j = 0; $j <= $i; $j++) {
									echo $parts[$j]."/";
	//								if(!($j==$i)) {
	//									$slash = "/";
	//								} else {										
	//									$slash = "";
	//								}
									//echo str_replace($dirNames, $linkNames, $parts[$j]).$slash;
								}
								//change parent directory name for shopping cart files

								$parts[$i] = str_replace($cartVersion, $cartBreadCrumbName, $parts[$i]);
								echo("\">". str_replace('-', ' ', ucwords($parts[$i]))."</a> <span class='breadCrumbsSeparator'>&rang;</span> ");
							} else {
								//it the file name (no link)
								$str = $parts[$i];
								$pos = strrpos($str,".");
								$parts[$i] = substr($str, 0, $pos);
								echo str_replace('-', ' ', $parts[$i]);
							}
						} //end for
					}
				?>
			</div>
		  </div>
		</div>
	   <?php endif;?>

	</div>
	<!-- InstanceBeginEditable name="SlideShow" -->

<!-- InstanceEndEditable -->
  
<div class="container-fluid">
  <div class="row text-center">
    <div><!-- InstanceBeginEditable name="Title" -->
    <!-- InstanceEndEditable --> 
		
	  <?php if($new_product == 'true'):?> 
		<h1 class="inlineBlock onSaleInv m-r-1"><strong>&nbsp;NEW!&nbsp;</strong></h1>
	  <?php endif;?>  
	  <?php if(isset($on_sale) && ($on_sale == "true")):?>
		<h1 class=" inlineBlock m-r-1"> - <span class="text-danger bg-danger text-nowrap">&nbsp;On Sale!&nbsp;</span></h1>
	  <?php endif;?>    
	  <?php if(isset($usePinterest) && ($usePinterest == "true")):?>         
          <a class="pinItButton" data-pin-do="buttonPin" data-pin-tall="true" href="https://www.pinterest.com/pin/create/button/?url=<?=urlencode($pageURL);?>&media=<?=urlencode($mainItemPictureAbsolute)?>&description=<?=$pinterestDescription;?>"><img src="//assets.pinterest.com/images/pidgets/pinit_fg_en_rect_gray_28.png" /></a>
	  <br> 
      <?php endif?>
    
	  <?php if(isset($kit_discontinued) && ($kit_discontinued == "true")):?>    
         <div  class="well well-sm inlineBlock">&nbsp;&nbsp;&nbsp;<strong class="red">Discontinued!</strong> - Replaced by: <a href="<?=$baseURL;?><?=$kit_link_path;?><?=$kit_replacement_filename;?>"><strong>
                            <?=$kit_replacement;?>
                            </strong></a></div>
      <?php endif?>
    </div>
  </div>  <!-- InstanceBeginEditable name="MainContent" -->
  <div class="row" align="center">
  <!--webbot bot="PurpleText" PREVIEW="Do not remove this tag below.

				It tells us where the content of the cart template goes."-->
 
                  <?=$contents;?>
  </div>
  <!-- InstanceEndEditable -->
  <hr>
  <div class="row">
    <div class="text-center footer">
      <p> &copy; <?php echo date("Y");?> G3 Business Solutions, Inc. &middot; All Rights Reserved &middot;  <a href="../returns.php">Returns</a> &middot; <a href="../terms.php">Terms </a> &middot; <a href="../privacy.php">Privacy</a> &middot; <a href="../contact-us.php">Contact</a> &middot; <a href="../about.php">About</a></p>
    </div>
    <div class="footer right m-r-2"><em>Designed by <a href="http://mmdbiz.com/" target="_blank">MMDbiz</a></em></div>
  </div>
  <hr>
</div>
<!-- InstanceBeginEditable name="StructuredData" --><!-- #BeginLibraryItem "/Library/structDataContent.lbi" -->
<span itemscope itemtype="http://schema.org/<?=$itemtype;?>">
	<span itemprop="brand" itemscope itemtype="http://schema.org/Brand">
		<meta itemprop="name" content="DiscSox">
		<meta itemprop="logo" content="<?=$imageFolderPath;?>logos/ds_logo_68.png">
	</span>
	<meta itemprop="name" content="<?=$struct_detail_page_title;?>">
	<meta itemprop="image" content="<?=$mainItemPictureAbsolute;?>">
	<?php if (isset($struct_currentsku) && $struct_currentsku != "") :?>
        <meta itemprop="sku" content="<?=$struct_currentsku;?>" />
        <meta itemprop="gtin" content="<?=$struct_gtin;?>" />
        <?php if (isset($struct_shipping_weight) && $struct_shipping_weight != "") :?>
            <meta itemprop="weight" content="<?=$struct_shipping_weight;?>" />
            <meta itemprop="shipping_weight" content="<?=$struct_shipping_weight;?>">
        <?php else:?>
            <meta itemprop="weight" content="<?=$struct_weight;?>" />
            <meta itemprop="shipping_weight" content="<?=$struct_weight;?>">
        <?php endif;?>
        <span itemprop="offers" itemscope itemtype="http://schema.org/Offer">
            <meta itemprop="price" content="<?=$struct_retail_price;?>">
            <meta itemprop="priceCurrency" content="USD">
            <meta itemprop="url" content="<?=$pageURL;?>">
            <?php if($debug):?>
                <?php echo("display product: ". $struct_display_product);?>
                <?php echo("on_backorder: ". $struct_on_backorder);?>
                <?php echo("discontinued: ". $struct_discontinued);?>
            <?php endif;?>
            <?php $availability = "InStock";?>
            <?php if(isset($struct_display_product) && ($struct_display_product == "true")):?>
                <?php if(isset($struct_on_backorder) && ($struct_on_backorder == "true")):?>
                    <?php $availability = "BackOrder";?> 
                <?php endif;?>
                <?php if(isset($struct_discontinued) && ($struct_discontinued == "true")):?>
                    <?php $availability = "Discontinued";?> 
                <?php endif;?>
            <?php else:?>
                <?php $availability = "Discontinued";?>
            <?php endif;?>
                <link itemprop="availability" content="<?= 'https://schema.org/' . $availability;?>"/>
        </span>
    <?php endif;?>
	<meta itemprop="description" content="<?=$struct_description;?>">
</span><!-- #EndLibraryItem --><!-- InstanceEndEditable -->  
 <!-- Search Modal -->
  <div id="modalSearch" class="modal fade" role="dialog">
       <div class="modal-dialog">
           <!-- Modal content-->
           <div class="modal-content">
               <div class="modal-header red">
                   <button type="button" class="close" data-dismiss="modal">&times;</button>
                   <h4 class="modal-title"><strong>Search DiscSox</strong></h4>
               </div>
               <div class="modal-body">
                   <!-- Add the modal body here from https://www.google.com/cse -->
                   <script>
					(function() {
					  var cx = '002646908333670893476:ldvakzfrg2m';
					  var gcse = document.createElement('script');
					  gcse.type = 'text/javascript';
					  gcse.async = true;
					  gcse.src = 'https://cse.google.com/cse.js?cx=' + cx;
					  var s = document.getElementsByTagName('script')[0];
					  s.parentNode.insertBefore(gcse, s);
					})();
				  </script>
                  <gcse:search></gcse:search>
               </div>
               <div class="modal-footer">
                   <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
               </div>
           </div>
       </div>
    </div>
  <!-- modal skeleton -->
  <div id="modal" class="modal fade info-modal" role="dialog" aria-labelledby="how-to-info" aria-hidden="true">
      <div class="modal-dialog">
          <!-- /# content (header, body and footer) goes here -->
      </div>
  </div>
<?php if(!$production):?> 
    <!-- jQuery (necessary for Bootstrap's JavaScript plugins) --> 
    <script src="../_js/jquery-1.11.3.min.js"></script> 
        
    <!-- Include all compiled plugins (below), or include individual files as needed --> 
    <script src="../_js/bootstrap.js"></script>
        <!-- to switch Bootstrap dropdown from click to hover include the following:-->
    <!--<script src="_js/jquery.bootstrap-dropdown-hover.min.js"></script>-->
    <script> 
        //$('.navbar [data-toggle="dropdown"]').bootstrapDropdownHover({
        //});
        <!-- END Bootstrap dropdown click to hover-->
        
    //	  $('.dropdown-toggle').click(function() {
    //		  $(this).next('.dropdown-menu').slideToggle(500);
    //	  });
    //	  $('.navbar .dropdown').hover(function() {
    //		  $(this).find('.dropdown-menu').first().stop(true, true).slideDown();
    //	  }, function() {
    //		  $(this).find('.dropdown-menu').first().stop(true, true).slideUp();
    //	  });
     </script>
     <!-- SmartMenus jQuery plugin -->
    <script type="text/javascript" src="../_js/jquery.smartmenus.js"></script>    
    <!-- SmartMenus jQuery Bootstrap Addon -->
    <script type="text/javascript" src="../_js/jquery.smartmenus.bootstrap.js"></script>
    <!-- LightBox Bootstrap Addon -->
	<script type="text/javascript" src="../_js/ekko-lightbox.js"></script>
	<script type="text/javascript" src="../_js/jquery.simpleGallery.js"></script>
    <script type="text/javascript" src="../_js/jquery.simpleLens.js"></script>
<?php else:?>
	<script src="../_js/allscripts.min.js"></script>
<?php endif;?>
<!--Pinterest Script-->
<?php if(isset($usePinterest) && ($usePinterest == "true")):?> 
	<script async defer src="//assets.pinterest.com/js/pinit.js"></script>
<?php endif;?>
<!-- InstanceBeginEditable name="ExtraScripts" -->
<script src="../_js/validator.min.js"></script>
 <script>  
	$(document).ready(function() {
		
		// add spinner to Place Secure Order button but only if not disabled
		$('.submit-order-button').on('click', function() {
			var $this = $(this);
			if (!$(this).hasClass('disabled')) {

			  $this.button('loading');
				setTimeout(function() {
				   $this.button('reset');
			   }, 20000);
			}
		});
		
		// ----- Form validation and show/hide stuff -----//
		
//		var resetContent;
//		var loginContent;
		var loginStuff;
		var existingCustContent;
	    var validatorResult =true;
		//remove reset PWD input
		//resetContent = $( "#resetPWD" ).detach();
		loginStuff = $( "#resetPWD" ).detach();
		//remove login input
		//loginContent = $( "#regLogin" ).detach();
		
		//Clicked on forgot password
		$('#forgotPWD').click(function(){	
			//add reset PWD content
			//resetContent.appendTo( "#login-form" );
			//resetContent = null;
			//loginStuff.appendTo( "#login-form" );
			loginStuff.appendTo( ".loginWrapper" );
			//remove regular login
			loginStuff = $( "#regLogin" ).detach();
			$("#message, #resetMessage, #backToLogin").toggleClass("hide");
			$('#reset-form').validator();
		});
		//got to forgot password via link
		if(window.location.hash == "#forgotPWDbyLink"){
			//add reset PWD content
			//resetContent.appendTo( "#login-form" );
			//resetContent = null;
			//loginStuff.appendTo( "#login-form" );
			loginStuff.appendTo( ".loginWrapper" );
			//remove regular login
			//loginContent = $( "#regLogin" ).detach();
			loginStuff = $( "#regLogin" ).detach();
			$("#message, #resetMessage, #backToLogin").toggleClass("hide");
			$('#reset-form').validator();
		}
		
		//Clicked on back to login
		$('#backToLogin').click(function(){	
			//add regular login
			//loginContent.appendTo( "#login-form" );
			//loginContent = null;	
			//loginStuff.appendTo( "#login-form" );
			loginStuff.appendTo( ".loginWrapper" );
			//remove reset PWD content
			//resetContent = $( "#resetPWD" ).detach();
			loginStuff = $( "#resetPWD" ).detach();
			$("#message, #resetMessage, #backToLogin").toggleClass("hide");
			$('#login-form').validator();
		});
				
		//Clicked on continue on new customers in checkout
		$('#newCustbutton').click(function(){	
			//remove existing Customers content
			existingCustContent = $( "#existingCustomers" ).detach();
			//show new customer fields
			$("#newCustButtonContainer, #centerDividerLeft, #billingTitle, #billingAddrBackTop, #billingAddrContinueTop" ).toggleClass("hide");
			$("#newCustDetailsLeft, #newCustDetailsRight" ).toggleClass("moveAway");
		});
		
		//Clicked on Back on Top of Billing fields in checkout
		$('#billingAddrBackTop').click(function(){	
			//go back to previous form layout	
			existingCustContent.prependTo( "#custInfoContainer" );
			existingCustContent = null;
			$("#newCustButtonContainer, #centerDividerLeft, #billingTitle, #billingAddrBackTop, #billingAddrContinueTop" ).toggleClass("hide");
			$("#newCustDetailsLeft, #newCustDetailsRight" ).toggleClass("moveAway");
		});
		//Clicked on Back on Bottom of Billing fields in checkout
		$('#billingAddrBack').click(function(){	
			//go back to previous form layout	
			existingCustContent.prependTo( "#custInfoContainer" );
			existingCustContent = null;
			$("#newCustButtonContainer, #centerDividerLeft, #billingTitle, #billingAddrBackTop, #billingAddrContinueTop" ).toggleClass("hide");
			$("#newCustDetailsLeft, #newCustDetailsRight" ).toggleClass("moveAway");
		});
		
		;//Clicked on continue on top of Billing Address in checkout
		$('#billingAddrContinueTop').click(function(){	
			// check form		
			stateValidator();
			$('form').validator('validate');
			//see if we have errors
			if ($(".has-error").length > 0) {
			  //alert ("Please correct errors! ");
			} else {
			  //alert ("Form is good! ");
			  //"hide" billing address stuff , 
			  $("#billingTitle, #shippingTitle, #billingAddrBackTop, #billingAddrContinueTop, #custShipFieldsBackTop, #custShipFieldsContinueTop" ).toggleClass("hide");
			  $("#newCustomers, #newCustDetailsLeft, #newCustDetailsRight, #NewCustPayMethod, #custShipFields" ).toggleClass("moveAway");
			}
		});
		//Clicked on continue on bottom of Billing Address in checkout
		$('#billingAddrContinue').click(function(){	
			// check form		
			stateValidator();
			$('form').validator('validate');
			//see if we have errors
			if ($(".has-error").length > 0) {
			  //alert ("Please correct errors! ");
			} else {
			  //alert ("Form is good! ");
			  //"hide" billing address stuff , 
			  $("#billingTitle, #shippingTitle, #billingAddrBackTop, #billingAddrContinueTop, #custShipFieldsBackTop, #custShipFieldsContinueTop" ).toggleClass("hide");
			  $("#newCustomers, #newCustDetailsLeft, #newCustDetailsRight, #NewCustPayMethod, #custShipFields" ).toggleClass("moveAway");
			}
		})
			
		//Clicked on Continue on Top of shipping fields in checkout
		$('#custShipFieldsContinueTop').click(function(){
			//alert ("Clicked Continue! ");	
			//Submit Form
		  $( "#Form3" ).submit();
		});	
		
		//Clicked on Back on Top of shipping fields in checkout
		$('#custShipFieldsBackTop').click(function(){	
			//go back to previous form layout
		  $("#billingTitle, #shippingTitle, #billingAddrBackTop, #billingAddrContinueTop, #custShipFieldsBackTop, #custShipFieldsContinueTop" ).toggleClass("hide");
		  $("#newCustomers, #newCustDetailsLeft, #newCustDetailsRight, #NewCustPayMethod, #custShipFields" ).toggleClass("moveAway");
		});
		//Clicked on Back on bottom of shipping fields in checkout
		$('#custShipFieldsBack').click(function(){	
			//go back to previous form layout
		  $("#billingTitle, #shippingTitle, #billingAddrBackTop, #billingAddrContinueTop, #custShipFieldsBackTop, #custShipFieldsContinueTop" ).toggleClass("hide");
		  $("#newCustomers, #newCustDetailsLeft, #newCustDetailsRight, #NewCustPayMethod, #custShipFields" ).toggleClass("moveAway");
		});
			
			
//		$('form').on('valid.bs.validator invalid.bs.validator', function (e) {
//		  if (e.relatedTarget.name === 'userName') {
//			e.type === 'valid' ? console.log("valid!") : console.log("invalid!")
//		  }
//		})
	
		
//			$('form').on('valid.bs.validator', function (e) {
//				  validatorResult = true;
//		    //alert( "form valid, name : " + e.relatedTarget.name);
//			});
//			$('form').on('invalid.bs.validator', function () {
//		    //alert( "form invalid, name : " + e.relatedTarget.name );
//			//alert (validatorResult);
//			});
			
			
		function  stateValidator() {
		  $('form').validator({
			custom: {
			  stateprov: function($el) {
				var value = $el.val();
				//stateprov = $el.attr('stateprov');
				//return (value === stateprov) ? false : true;
				if (value === "INVALID") {
				  return false;
				} else {
				  return true;
				}						
			  }
			},
			errors: {
				stateprov: "Select a state!"
			}
		  });
		}
		
		$( "#stateSelect" ).change(function() {
		  //alert( "Handler for stateSelect called." );		  
		  stateValidator();
		});
		
		$( "#countrySelect" ).change(function() {
			//alert( "Handler for countrySelect called." );
			//check for state being selected 
			stateValidator();
		});

		// ----- End Form validation and show/hidE stuff -----//
		
        //update cart when drop down values change
		$( "select#cartDD" ).change(function() {
		  document.forms["Form1"].submit();
		});
		//show the update icon on input change
		$("input[type='text']").on('input',function(e){
		 //alert('Changed!');
		   //$(".updateQty" ).removeClass("hide");
		   $( this).next().removeClass("hide");
		   //$(".updateQty" ).tooltip('show');
		   $( this).next().tooltip('show');
		});
        //update cart when click on update
		$( ".updateQty" ).click(function() {
			//alert("update Qty clicked");
		  document.forms["Form1"].submit();
		});
    });
</script>
<!-- InstanceEndEditable -->
  <script>  
	var dirPrefix = "<?php echo $dirPrefix; ?>";
	var includeAnimateStuff = "<?php echo $inclAnimation; ?>";
	var useToolTip = "<?php echo $useToolTip; ?>";
	var useZoom = "<?php echo $useZoom; ?>";
	var useGallery = "<?php echo $useGallery; ?>";
	var loadingGif = '_images/helpers/loading.gif';
	var loadingGifPath = dirPrefix + loadingGif;
		var imageFolderPath = "<?php echo $imageFolderPath; ?>";
    $(document).ready(function(){
          $(document).delegate('*[data-toggle="lightbox"]', 'click', function(event) {
			  event.preventDefault();
			  $(this).ekkoLightbox(
			  );
		  });
		  if (useToolTip == "1") {
			  $('[data-toggle="tooltip"]').tooltip();
		  }
		  //only initialize the zoom if it is NOT a mobile device and when set to enabled 
		  if ((!(isMobile.any)) && (useZoom == "1")) {
			  $('#demo-1 .simpleLens-big-image').simpleLens({
				  loading_image:  loadingGifPath
			  });
		  }
		  //initialize the thumbnail gallery when set to enabled
		  if (useGallery == "1") {
			  $('#demo-1 .simpleLens-thumbnails-container img').simpleGallery({
				  loading_image: loadingGifPath
			  });
		  }
		  //Opening accordion panels from other files
		  location.hash && $(location.hash + '.collapse').collapse('show');
		  //if the accordion is in the same file then we need a click funcion
//		  $(".openDrawerDivider").click(function(){
//			  $("#collapseTwo").collapse('show');
//		  });

    });
	// dynamic modal code
	$('a[rel=modal]').on('click', function(evt) {
    evt.preventDefault();
    var modal = $('#modal').modal();
    modal
        //.find('.modal-body')
        .find('.modal-dialog')
		
        .load($(this).attr('href'), function (responseText, textStatus) {
            if ( textStatus === 'success' || 
                 textStatus === 'notmodified') 
            {
                modal.show();
            }
    });
	
});
  </script>
  <script>
  (function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){
  (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
  m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
  })(window,document,'script','https://www.google-analytics.com/analytics.js','ga');

  ga('create', 'UA-1342362-1', 'auto');
  ga('send', 'pageview');

</script>
</body>
<!-- InstanceEnd --></html>
