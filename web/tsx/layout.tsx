/* eslint-disable no-unused-vars */
import React from "react"

// export
// enum Form{
//   Box,
//   Grid,
//   Stack
// }
// export
// enum Orientation{
//   Horizontal,
//   Vertical,
// }
export
interface IProps {
  //orientation : Orientation;
}

export default
class Layout extends React.Component<IProps>{   
  constructor(props : IProps){
    super(props);   
    //this.state = { dirs : this.props.dirs};
  }
  public render(){
          
    return <div style={{ borderRadius: "3px", overflow: "auto", maxHeight : clientHeight * 0.8 + "px" }}> 
            <Container className="col app-container"
                      style={{overflow: "auto", height: clientHeight}}>
              <Row noGutters={true} className="m-1 p-2"
                  style = {{  border: "1px solid #dbdbdb", borderRadius: "5px"}}>
                {
                !this.state.isCollapseNav ? 
                  <Col className="col-auto"> 
                    <Button size="md" className = {"icon-cog"} style = {buttonStyle}
                            onClick = {this.handleShowConfig}/>
                    <Button size="md" className = {"icon-doc"} style = {buttonStyle}
                            onClick = {this.handleAddGraph} />
                    <TreeNav scheme={this.state.navScheme}
                            onDoubleClick = { this.handleAddSignalOnGraph } />            
                  </Col>            
                  : ""
                }
                <Col className="col-auto" > 
                    <Button size="md" style = {{ paddingLeft : "0px", paddingRight : "0px",
                                                width : "25px", ...buttonStyle}} variant = "info"
                            onClick = {this.handleCollapseNav}> 
                            {this.state.isCollapseNav ? String.fromCharCode(187) : String.fromCharCode(171)}
                    </Button>
                </Col>
                <Col className="col"> 
                  <GraphPanelRedux listGraph = { this.state.listGraph } 
                                  onCloseGraph = { this.handleCloseGraph } />
                </Col>
              </Row>
            </Container>      
           </div>
  }  
  public addWidget(){

  }

}