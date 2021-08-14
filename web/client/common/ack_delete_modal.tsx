import React from "react";
import { Modal, Button } from "react-bootstrap";

interface IPropsAckDeleteModal { 
  show : boolean;
  description : string;
  title : string;
  onYes : () => any;  
  onHide : () => any;  
};
interface IStateAckDeleteModal { 
};

export default
class AckDeleteModal extends React.Component<IPropsAckDeleteModal, IStateAckDeleteModal>{  
  constructor(props : IPropsAckDeleteModal){
    super(props);    
    this.state  = { isShowBtn : false };   
  }   
  render(){  
    return (
      <Modal show={this.props.show} onHide={this.props.onHide}>
        <Modal.Header closeButton>
          <Modal.Title>{this.props.title}</Modal.Title>
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