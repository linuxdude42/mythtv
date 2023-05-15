import { Component, OnInit } from '@angular/core';
import { TranslateService } from '@ngx-translate/core';
import { MenuItem } from 'primeng/api';

@Component({
  selector: 'app-dashboard',
  templateUrl: './dashboard.component.html',
  styleUrls: ['./dashboard.component.css']
})
export class DashboardComponent implements OnInit {

  fullMenu: MenuItem[] = [
    { label: 'dashboard.backendStatus', routerLink: 'status' },
    { label: 'dashboard.channeleditor', routerLink: 'channel-editor' },
    { label: 'dashboard.programguide', routerLink: 'program-guide' },
    { label: 'dashboard.recordings', routerLink: 'recordings' },
  ]

  activeItem = this.fullMenu[0];

  constructor(private translate: TranslateService) {
    this.fullMenu.forEach(entry => {
      if (entry.label)
        this.translate.get(entry.label).subscribe(data =>
          entry.label = data)
    });
  }

  ngOnInit(): void {
  }

}
