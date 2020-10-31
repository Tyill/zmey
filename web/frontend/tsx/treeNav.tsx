/* eslint-disable no-unused-vars */
import React from "react"
import "../css/treeNav.css";
import "../css/fontello.css";

export
interface ITreeNavDir{
  name : string;
  files : Array<string>;
  subdirs : Array<ITreeNavDir>;
  isShow : boolean;
}
interface ITreeNavProps {
  dirs: Array<ITreeNavDir>;
}
interface ITreeNavState {
  dirs: Array<ITreeNavDir>;
}
export default
class TreeNav extends React.Component<ITreeNavProps, ITreeNavState>{   
  constructor(props : ITreeNavProps){
    super(props);   
    this.state = { dirs : this.props.dirs};
  }
  public render(){
    let objList : Array<JSX.Element> = [];
    for(let dir of this.props.dirs){
      this.addDir(0, dir, objList);    
      if (dir.isShow){
        this.renderDir(1, dir, objList);
        this.addFiles(1, dir, objList);
      }
    }       
    return <div style={{ borderRadius: "3px", overflow: "auto", maxHeight : 300 + "px" }}> 
             {objList}          
           </div>
  }  
  private renderDir(level : number, 
                    dir : ITreeNavDir,
                    objList : Array<JSX.Element>){    
    for (let sdir of dir.subdirs){
      this.addDir(level, sdir, objList);
      if (sdir.isShow){
        this.renderDir(level + 1, sdir, objList);
        this.addFiles(level + 1, sdir, objList);
      }
    }    
  }
  private addDir(level : number, 
                 dir : ITreeNavDir,
                 objList : Array<JSX.Element>){
    objList.push(
      <section key={level + "." + dir.name}
               className="treeNav-dir"
               style={{ paddingLeft: level * 10 }}
               onClick= { (e) => {dir.isShow = !dir.isShow;
                                 this.setState({ dirs : this.props.dirs});} }>        
        <span className = { dir.isShow ? "icon-down-dir" : "icon-left-dir" }/>
        {dir.name}
      </section>);
  }
  private addFiles(level : number, 
                   dir : ITreeNavDir,
                   objList : Array<JSX.Element>){    
    for (let f of dir.files){      
      objList.push(
        <section key={level + "." + f} 
                 className="treeNav-file"
                 draggable="true"
                 style={{ paddingLeft: 5 + level * 10 }} >
          {f}
        </section>);
    }
  }
}