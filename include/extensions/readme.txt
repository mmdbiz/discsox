

Overview:
All of the functionality in the shopping cart has been built using PHP class libraries.
By using class libraries to define the functionality of the cart, we can also modify the
way the specific functions work by adding custom class libraries. This allows us 
to redefine how a specific function works or provide additional functionality without 
changing any of the original class files.

The Extensions section of the control panel allows you to set custom class file names. 
Just ftp your custom class library file into the "include/extensions" directory and
enter your extended class filename in the control panel. It will then be automatically
loaded at run time.

An example entry would be:  discounts: custom.discount.inc

This says that the custom.discount.inc file in the extensions directory has a
"x_Discounts" class in it that extends the default "Discounts" class 
functionality. The x stands for extension. Simple as that. You can then modify
the specific functions that you would like changed and leave alone the ones
you don't want to change.

Example class file:

<?php
class x_Discounts Extends Discount{

    // Class-wide variables
    var $debug = false;

    // ----------------------------------------------------------------
    // Class constructor
    function x_Discounts(){

    }
    
    // ----------------------------------------------------------------
	// This functions overrides the default calculateDiscount function
	// in the discounts.inc
    function calculateDiscount($subtotal,$quantity){
    
		// your new code here...
    
    }
}
?>




