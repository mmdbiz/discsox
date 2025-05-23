/*
Misc javascript functions used throughout the cart
*/

// -------------------------------------------------------------------
function GoTo(thisURL) {
     //alert(thisURL);
     location = thisURL;
}
// -------------------------------------------------------------------
function OpenWindow(thisURL,x,y) {

	alert(thisURL);

    var gWin;
    if(x == undefined){
        gWin = window.open(thisURL,'GraphicWindow','resizable=0','scrollbars=0');
    }
    else{
        gWin = window.open(thisURL,'GraphicWindow','width='+x+',height='+y+',resizable=0,scrollbars=0');
    }
}


function popup(img,h,w,title){

    var sWidth = screen.width / 2 - w;

     var win = window.open("","graphics","top=100,left=" + sWidth + ",scrollbars=0,resizable=1");
     win.resizeTo(w,h + 100);
     win.document.open("text/html", "replace");
     win.document.write("<html><head><title>"+title+"</title></head><body topmargin=0 leftmargin=0 marginwidth=0 marginheight=0>");
     win.document.write("<img src="+img+" width="+w+" height="+h+"><br>");
     win.document.write("<form><div align=center><input type=button name=Close value=\"Close Window\" onclick=window.close()></div></form>");
     win.document.write("</body></html>");
     win.status = "Zoom Image of " + title;
     win.focus();
}


// -------------------------------------------------------------------
function qtyCheck(form){

     var qtySelected = parseInt(0);

     for(i = 0; i < form.elements.length; i++){

          var fieldName = form.elements[i].name;

          if(fieldName.substring(0,8).toLowerCase() == "quantity"){
               var fieldValue = parseInt(form.elements[i].value);

               if(fieldValue > 0){
                    qtySelected += parseInt(fieldValue);

                        // Check availability

                    var id = fieldName.substring(9,fieldName.length);
                    var availFld = "available|" + id;
                    var minField = "minimum|" + id;

                    var nameFld = "name|" + id;
                    var productName = form.elements[nameFld].value

                    var pattern = /\s{2,}/g;
                    var pName = productName.replace(pattern,"");


                    if(eval(form.elements[availFld])){
                        var qtyAvailable = parseInt(form.elements[availFld].value);
                        if(fieldValue > qtyAvailable){
                            if(qtyAvailable <= 0){
                                alert("Sorry, the '" + pName + "' is not available at this time.");
                            }
                            else{
                                alert("There are only " + qtyAvailable + " of the '" + pName + "s' available.\n\n" +
                                      "Please reduce the quantity so it is equal to or fewer than " + qtyAvailable + " item(s).");
                            }
                            form.elements[i].value = "";
                            form.elements[i].focus();
                            return false;
                        }
                    }
                    if(eval(form.elements[minField])){
                        var qtyMinimum = parseInt(form.elements[minField].value);
                        if(fieldValue < qtyMinimum){
                            alert("You must purchase a minimum of " + qtyMinimum + " '" + pName + "s'.");
                            form.elements[i].value = "";
                            form.elements[i].focus();
                            return false;
                        }
                    }
               }
          }
     }

     if(qtySelected == 0){
          alert("You have not entered a quantity of an item to add to the cart.");
          return false;
     }

return true;
}


// -------------------------------------------------------------------
// Checks quantity, inventory and option selections
function checkInputs(form,sku){

	var isInventoried = false;
	var inventoryFld = "item." + sku + ".inventory";
	var inventoryQty = 0;
	var minimumQty = 0;
	if(typeof(form.elements[inventoryFld]) != 'undefined'){
		isInventoried = true;
		inventoryQty = form.elements[inventoryFld].value;
		var minFld = "item." + sku + ".minimum";
		if(typeof(form.elements[minFld]) != 'undefined'){
			minimumQty = form.elements[minFld].value;
		}
	}

	// It's inventoried, get selected quantity
	var qtyFld = "item." + sku + ".quantity";
    var qtySelected = parseInt(0);

    // select box
	if(form.elements[qtyFld].type == "select-one"){
		var qIndex = form.elements[qtyFld].selectedIndex;
		qtySelected = parseInt(form.elements[qtyFld].options[qIndex].value);
	}
    else{
	    // text box
		qtySelected = parseInt(form.elements[qtyFld].value);
	}

	if(isNaN(qtySelected) || qtySelected < 1){
		alert("You have not entered a valid quantity for this item?");
		form.elements[qtyFld].value = "";
		form.elements[qtyFld].focus();
		return false;
	}

	if(isInventoried && (qtySelected > inventoryQty)){
		alert("You entered a quantity for this item that is greater than the available quantity (" +  inventoryQty + ")");
		form.elements[qtyFld].value = "";
		form.elements[qtyFld].focus();
		return false;
	}
	if(minimumQty > 0 && (qtySelected < minimumQty)){
		alert("You must purchase a minimum of " + minimumQty + " items");
		form.elements[qtyFld].value = "";
		form.elements[qtyFld].focus();
		return false;
	}

	// Now get all options and check against the matrix
	var optionFld = "option." + sku + ".";
	var nameLen = sku.length + 8;

	var Options = new Array();

	for(i = 0; i < form.elements.length; i++){

		var fieldName = form.elements[i].name;
		var fieldType = form.elements[i].type;

		if(typeof(fieldName) != 'undefined' && fieldName.substring(0,nameLen) == optionFld){

			var fldValue = null;

			if(fieldType == "select-one"){
	            var selectedIndex = form.elements[fieldName].selectedIndex;
				fldValue = form.elements[fieldName].options[selectedIndex].value;
			}
			if(fieldType == "text"){
				fldValue = form.elements[fieldName].value;
			}

            if(fieldType == 'radio'){
                var isChecked = false;

                for(j=0; j < form.elements[fieldName].length;j++){
                    if(form.elements[fieldName][j].checked == true){
                        isChecked = true;
                        break;
                    }
                }

                if(!isChecked){
                    var parts = fieldName.split('.');
                    fldValue = "invalid|" + parts[2];
                }
            }


			// Stop here if we find an invalid option
			if(fldValue && fldValue.substring(0,7).toLowerCase() == "invalid"){
				// find the 2 parts of the value
				testFlds = fldValue.split('|');
				var optionName = testFlds[1];
				var message = "You must select an option for '" + optionName +
							  "' before you can add this item to your cart.";
				alert(message);
				if(fieldType == "radio"){
					form.elements[fieldName][0].focus();
				}
				else{
					form.elements[fieldName].focus();
				}
				return false;
			}
			else{
				if(fldValue){
					var flds = fldValue.split('|');
					Options.push(flds[0]);
				}
			}
		}
    }

	if(Options.length > 0){
		var strOptions = Options.join(':');
		return checkInventoryOptions(form,qtyFld,qtySelected,sku,strOptions);
	}

return true;
}

// -------------------------------------------------------------------
function showIt(whichEl,classEL){
    if(document.all){
        whichEl = document.all[whichEl];
        classEL = document.all[classEL];
    }
    else{
        whichEl = document.getElementById(whichEl);
        classEL = document.getElementById(classEL);
    }
    whichEl.style.display = (whichEl.style.display == "none" ) ? "" : "none";
    classEL.className = (classEL.className == "subListItem" ) ? "subListItemClicked" : "subListItem";
}

// -------------------------------------------------------------------
function inventoryList(){
	this.length = 0;
	this.items = new Array();
	this.getItem = function(in_key) {
		return this.items[in_key];
	}
	this.setItem = function(in_key, in_value) {
		if (typeof(in_value) != 'undefined') {
			if (typeof(this.items[in_key]) == 'undefined') {
				this.length++;
			}

			this.items[in_key] = in_value;
		}
		return in_value;
	}
	this.hasItem = function(in_key){
		return typeof(this.items[in_key]) != 'undefined';
	}
}

// -------------------------------------------------------------------
function checkInventoryOptions(form,qtyFld,qtySelected,sku,strOptions){

	if(inventory.length > 0 && inventory.hasItem(sku)){

		for(j=0;j<inventory.items[sku].length;j++){

			var min = inventory.items[sku][j][1];
			var available = inventory.items[sku][j][2];

			if(inventory.items[sku][j][0] != "" && strOptions && inventory.items[sku][j][0] == strOptions){
				if(qtySelected > available){
					var qtyToEnter = parseInt(available) + 1;
					alert("The available quantity for this item with the selected options is (" +  available + ")\n\n" +
						  "Enter a quantity lower than " +  qtyToEnter + " or select different options to try again.");
					form.elements[qtyFld].value = "";
					form.elements[qtyFld].focus();
					return false;
				}
				if(min > 0 && qtySelected < min){
					alert("You must purchase a minimum of (" + min + ") items with the selected options");
					form.elements[qtyFld].value = "";
					form.elements[qtyFld].focus();
					return false;
				}
			}
		}
	}
	return true;
}

// -------------------------------------------------------------------
function displayInventoryCount(form,sku){

	if(inventory.length > 0 && inventory.hasItem(sku)){

		var optionFld = "option." + sku + ".";
		var nameLen = sku.length + 8;

		var Options = new Array();

		for(i = 0; i < form.elements.length; i++){

			var fieldName = form.elements[i].name;
			var fieldType = form.elements[i].type;

			if(typeof(fieldName) != 'undefined' && fieldName.substring(0,nameLen) == optionFld){

				var fldValue = null;

				if(fieldType == "select-one"){
					var selectedIndex = form.elements[fieldName].selectedIndex;
					fldValue = form.elements[fieldName].options[selectedIndex].value;
				}
				if(fieldType == "text"){
					fldValue = form.elements[fieldName].value;
				}
				if(fieldType == 'radio'){
					for(j=0; j < form.elements[fieldName].length;j++){
						if(form.elements[fieldName][j].checked == true){
							var values = form.elements[fieldName][j].value.split('|');
							fldValue = values[0];
							break;
						}
					}
				}
				if(fldValue){
					var flds = fldValue.split('|');
					Options.push(flds[0]);
				}
			}
		}

		if(Options.length > 0){
			var strOptions = Options.join(':');
			for(j=0;j<inventory.items[sku].length;j++){
				var min = 0;
				var available = 0;
				if(inventory.items[sku][j][0] != "" && strOptions && inventory.items[sku][j][0] == strOptions){
					min = inventory.items[sku][j][1];
					available = inventory.items[sku][j][2];
					if(parseInt(min) > 0){
						var minFld = sku + '.minimum';
						var minDiv = document.getElementById(minFld);
						//alert(typeof(availableDiv));
						if(typeof(minDiv) != 'undefined'){
							minDiv.innerHTML = "<p><b>Minimum Purchase Quantity:</b> " + min;
						}
					}
					if(parseInt(available) > 0){
						var availableFld = sku + '.available';
						var availableDiv = document.getElementById(availableFld);
						//alert(typeof(availableDiv));
						if(typeof(availableDiv) != 'undefined'){
							availableDiv.innerHTML = "<p><b>Available Quantity:</b> " + available;
						}
					}
				}
			}
		}
	}
}





