<?php
$_isAdmin = true;
$_adminFunction = "configuration";

// initialize the program and read the config(s)
include_once("../include/initialize.inc");
$init = new Initialize();

global $_Registry;
$login = $_Registry->LoadClass("admin_login");
$login->CheckLogin();

if(count($_REQUEST) == 0){
    $_Common->redirect("config.welcome.php");
    exit;
}

$refreshMenu = false;

foreach(array_keys($_REQUEST) as $i=>$key){

    $RUN = false;
    switch($key){
        case "addtoconfig":
            $_Config->addtoSection();
            $RUN = 1;
            break;
        case "displayMenu":
            $_Config->displayMenu();
            $RUN = 1;
            break;
        case "editconfig":
            $_Config->getSection();
            $RUN = 1;
            break;
        case "updateconfig":
            $_Config->updateSection();
            $_Config->getSection();
            $RUN = 1;
            break;
        case "addentry":
            $_Config->addEntry();
            $_Config->getSection();
            $RUN = 1;
            break;
            
    } # End switch

    if($RUN){
        break;
    }

}
    # If some other function was tried,
    # just show the home page.

if(!$RUN){
    $_Common->redirect("config.welcome.php");
}

exit;

?>