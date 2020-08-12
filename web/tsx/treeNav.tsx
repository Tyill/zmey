/* eslint-disable no-unused-vars */
import React from "react"
import "../css/treeNav.css";
import "../css/fontello.css";


export default
class TreeNav extends React.Component{
   
  constructor(props){
    super(props);   
  
    this.state = { scheme : this.props.scheme};
  }
 
 
  renderSubmenu(level, submenu, 
     obj, outObjList, outMenuList){
   
    if (typeof(obj) === "string"){
     
      outObjList[submenu].push(<section key={level + "." + obj} 
                                 className="treeNav-row"
                                 draggable="true"
                                 style={{ paddingLeft: 10 + level * 5 }}
                                 onDragStart = {(e) => e.dataTransfer.setData('text', submenu + obj) }
                                 onDoubleClick = { (e) => this.props.onDoubleClick(obj + submenu.slice(0, -1)) } >
                                   {obj}
                               </section>);      
    }  
    else{
      submenu += obj.submenu + ".";
      
      outObjList[submenu] = [];

      outMenuList[submenu] = (<section key={level + "." + obj.submenu}
                               className="treeNav-head"
                               style={{ paddingLeft: 10 + level * 5,
                                        backgroundColor : (obj.isActive ? "silver" : "rgb(200, 20, 20)") }}
                               onClick= { (e) => {obj.isShow = !obj.isShow;
                                                 this.setState({ scheme : this.props.scheme});} }> 
                                 {obj.submenu} 
                               <span className= { obj.isShow ? "icon-down-dir" : "icon-left-dir" } />
                              </section>);      
      if (obj.isShow){
        for(let o of obj.items)           
          this.renderSubmenu(level + 1, submenu, o, outObjList, outMenuList)
      }
    }  
  }

  render(){

    let objList = {},
        menuList = {};

    for(let obj of this.props.scheme)
      this.renderSubmenu(0, "", obj, objList, menuList);

    const clientHeight = document.documentElement ? document.documentElement.clientHeight : 300;
    
    let renderObj = [];
    for (let key in menuList){
      
      renderObj.push(menuList[key]);

      if (objList[key].length > 0)  
        renderObj.push(
         <div key={key} style={{ overflow: "auto", maxHeight : clientHeight * 0.8 + "px" }} >
          {objList[key]}
         </div>      
        );
    }
     
    return <div style={{ borderRadius: "3px", overflow: "auto" }}> 
              {renderObj}          
           </div>
  }
}
