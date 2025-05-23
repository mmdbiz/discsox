

// -------------------------------------------------------------------
function OpenQtyWindow(fld){
    var winQuery = "templates/qty.ranges.html?fldid=" + fld;
	var qtyWindow = window.open(winQuery,"_blank","toolbar=no,scrollbars=no,resizable=no,width=300,height=475,screenX=25,screenY=75,top=25,left=75");

    if(!qtyWindow.opener){
        qtyWindow.opener = self;
    }
return false;
}

// -------------------------------------------------------------------
function RelatedListWindow(pid,pname,sessid){
    var winQuery = "related.php?list_related=true&pid=" + pid + "&pname=" + pname;
	var qtyWindow = window.open(winQuery,"_blank","toolbar=no,scrollbars=yes,resizable=yes,width=600,height=400,screenX=25,screenY=75,top=25,left=75");

    if(!qtyWindow.opener){
        qtyWindow.opener = self;
    }
return false;
}

// -------------------------------------------------------------------
function shipDefineWindow(fld,shipper){
    var winQuery = "ship.define.php?fldid=" + fld + '&shipper=' + shipper;
	var sdWindow = window.open(winQuery,"_blank","toolbar=no,scrollbars=yes,resizable=yes,width=600,height=475,screenX=25,screenY=75,top=25,left=75");
    if(!sdWindow.opener){
        sdWindow.opener = self;
    }
return false;
}


// -------------------------------------------------------------------
function OptionListWindow(pid,pname,sessid){
    var winQuery = "options.php?list=true&pid=" + pid + "&pname=" + pname;
	var qtyWindow = window.open(winQuery,"_blank","toolbar=no,scrollbars=yes,resizable=yes,width=600,height=400,screenX=25,screenY=75,top=25,left=75");

    if(!qtyWindow.opener){
        qtyWindow.opener = self;
    }
return false;
}

// -------------------------------------------------------------------
function testEntries(form){

	var catFld = "catid[]";

	var flds = new Array(catFld,'sku','name','price');
	var missing = new Array();

	for(i=0;i<flds.length;i++){
		var fldName = flds[i];
		if((fldName.substring(0,5) == 'catid') && (form.elements[fldName].selectedIndex == -1)){
			missing.push('category');
		}
		else{
			if(fldName == 'sku' && form.elements[fldName].value.indexOf(' ') != -1){
				alert("Spaces are not allowed in the sku");
				form.elements[fldName].focus();				
				return false;
			}
			if(form.elements[fldName].value == ""){
				missing.push(fldName);
			}
		}
	}

	if(missing.length > 0){
		var missingFlds = missing.join(", ");
		alert("The following fields are required to add or update a product:\n\n" + missingFlds);
		return false;
	}

return true;
}


// -------------------------------------------------------------------
function autofillText(form,fldNum){
     var fill = true;
     if(form.elements['autofill_text'][1].checked){
         fill = false;
     }
     if(fill){
         var valueFld = "value[" + fldNum + "]";
         var prcFld = "price[" + fldNum + "]";
         var txtFld = "text[" + fldNum + "]";
         var name = form.elements[valueFld].value;
         var price = form.elements[prcFld].value;
         var addTxt = form.elements['add_text'].value;
         if(parseInt(strip(price)) > 0){
             form.elements[txtFld].value = name + " " + addTxt + " " + price;
             //form.elements[prcFld].value = strip(form.elements[prcFld].value);
         }
         else{
             form.elements[txtFld].value = name
         }
         //set order default
         var orderFld = "sequence[" + fldNum + "]";
         var order = form.elements[orderFld].value;
         if(order == ""){
			form.elements[orderFld].value = 1;
         }
     }
}


// -------------------------------------------------------------------
function clearOptionRow(form,fldNum) {

	var autoFillSelection = parseInt(0);
	if(form.elements['autofill_text'][1].checked){
		autoFillSelection = 1;
	}


	form.elements['autofill_text'][1].checked = true;

	var odidFld = "odid[" + fldNum + "]";
	var seqFld = "sequence[" + fldNum + "]";
	var valueFld = "value[" + fldNum + "]";
	var prcFld = "price[" + fldNum + "]";
	var txtFld = "text[" + fldNum + "]";
	var weightFld = "weight[" + fldNum + "]";

	form.elements[odidFld].value = "";
	form.elements[seqFld].value = "";
	form.elements[valueFld].value = "";
	form.elements[prcFld].value = "";
	form.elements[weightFld].value = "";
	form.elements[txtFld].value = "";

	form.elements['autofill_text'][autoFillSelection].checked = true;

}



// -------------------------------------------------------------------
function strip(val) {

	// Strips the dashes, spaces, etc
	// from the credit card number

	val = "" + val;
	if (!val)
		return "";
	var result = "";
	for (i=0; i < val.length; i++) {
		character = val.charAt(i);
		if ("0123456789.".indexOf(character) != -1)
		result += character;
	}
return result;
}



