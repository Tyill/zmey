import React from "react";
import { Modal, Button } from "react-bootstrap";

export
interface IAckDeleteDialog {
  description : string;
  title : string;
  show : boolean;
  onYes : ()=>any;
  onHide :()=>any;
};

interface IProps extends IAckDeleteDialog { 
};

interface IState { 
};

export default
class AckDeleteModal extends React.Component<IProps, IState>{  
  constructor(props : IProps){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){  
    return (
      <Modal show={this.props.show} onHide={this.props.onHide}>
        <Modal.Header closeButton>
          <Modal.Title className="unselectable">{this.props.title}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <p>{this.props.description}</p>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" style={{minWidth:"100px"}} onClick={this.props.onYes}>Yes</Button>
          <Button variant="primary" style={{minWidth:"100px"}} onClick={this.props.onHide} >No</Button>
        </Modal.Footer>
      </Modal>
    )
  }
}